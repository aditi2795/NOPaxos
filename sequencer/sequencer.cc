// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * sequencer/sequencer.cc:
 *   End-host network sequencer implementation.
 *
 * Copyright 2017 Jialin Li <lijl@cs.washington.edu>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************************/

#include <iostream>
#include <fstream>
#include "lib/message.h"
#include "lib/configuration.h"
#include "sequencer/sequencer.h"

using namespace std;

namespace sequencer {

Sequencer::Sequencer(uint64_t sequencer_id) : sequencer_id(sequencer_id) { }

Sequencer::~Sequencer() { }

uint64_t
Sequencer::Increment(uint32_t groupIdx) {
    if (this->counters.find(groupIdx) == this->counters.end()) {
        this->counters.insert(make_pair(groupIdx, 0));
    }

    return ++this->counters[groupIdx];
}

Configuration::Configuration(ifstream &file) {
    while (!file.eof()) {
        string line;
        getline(file, line);

        // Ignore comments
        if ((line.size() == 0) || (line[0] == '#')) {
            continue;
        }

        char *cmd = strtok(&line[0], " \t");

        if (strcasecmp(cmd, "interface") == 0) {
            char *arg = strtok(nullptr, " \t");
            if (!arg) {
                Panic("'interface' configuration line requires an argument");
            }

            char *iface = strtok(arg, "");

            if (!iface) {
                Panic("Configuration line format: 'interface name'");
            }

            this->interface = string(iface);
        } else {
            Panic("Unknown configuration directive: %s", cmd);
        }
    }
}

Configuration::~Configuration() { }

string
Configuration::GetInterface() {
    return this->interface;
}

Transport::Transport(Sequencer *sequencer, Configuration *config, specpaxos::Configuration *global_config)
    : sequencer(sequencer), config(config), global_config(global_config), sockfd(-1)
{
    struct sockaddr_ll sll;
    int sockopt = 1;

    if ((this->sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETHER_TYPE))) == -1) {
        Panic("Failed to open socket");
    }

    memset(&ifopts, 0, sizeof(ifopts));
    strncpy(ifopts.ifr_name, config->GetInterface().c_str(), IFNAMSIZ-1);
    if (ioctl(this->sockfd, SIOCGIFINDEX, &ifopts) < 0) {
        Panic("Failed to set ioctl option SIOCGIFINDEX: %s", strerror(errno));
    }

    if (setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) == -1) {
        Panic("Failed to set socket option SO_REUSEADDR");
    }

    bzero(&sll, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifopts.ifr_ifindex;

    if (bind(this->sockfd, (struct sockaddr *)&sll, sizeof(sll)) == -1) {
        Panic("Failed to bind socket");
    }

    fprintf(stderr, "Transport setup complete.");

    /* Sequencer sends out packets using multicast */
    /*for (int i = 0; i < config->g; i++) {
	for (int j = 0; j < config->n; j++) {
	    ReplicaAddress replica = config->replica(i, j);
	    int index = (i * config->n) + j;
	    this->destSockAddrs[index].sll_ifindex = ifopts.ifr_ifindex;
	    this->destSockAddrs[index].sll_halen = ETH_ALEN;
	    this->destSockAddr[index].sll_addr = replica.mac;
	}
    }*/
}

Transport::~Transport() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

void
Transport::Run() {
    int n;
    uint8_t buffer[BUFFER_SIZE];

    if (this->sockfd == -1) {
        Warning("Transport not registered yet");
        return;
    }

    while (true) {
        n = recvfrom(this->sockfd, buffer, BUFFER_SIZE, 0, nullptr, nullptr);

        if (n <= 0) {
            break;
        }

        if (ProcessPacket(buffer, n)) {
            for (int i = 0; i < global_config->g; i++) {
		for (int j = 0; j < global_config->n; j++) {
		    specpaxos::ReplicaAddress replica = global_config->replica(i,j);
    		    // Encapsulate packet.
                    uint8_t copied_packet[n + sizeof(ether_header) + sizeof(iphdr) + sizeof(udphdr)];
		    // Copy entire client packet into UDP payload.
		    memcpy(copied_packet + sizeof(ether_header) + sizeof(iphdr) + sizeof(udphdr), buffer, n);
		    // Copy current client headers (for modification).
		    memcpy(copied_packet, buffer, sizeof(ether_header) + sizeof(iphdr) + sizeof(udphdr));
		    // Set destination IP and UDP port in client packet.
		    SetPacketDest(copied_packet + sizeof(ether_header) + sizeof(iphdr) + sizeof(udphdr), &replica);
		    // Set source and destination IP and UDP port in client packet.
		    SetOuterPacketDestSrc(copied_packet, &replica);
		    // Format destination socket address.
		    struct sockaddr_ll sll;
		    SetSocketDest(&sll, &replica);
		    if (sendto(this->sockfd, copied_packet, n + sizeof(ether_header) + sizeof(iphdr) + sizeof(udphdr), 0,
                           (struct sockaddr*)&sll, // TODO: fix!!!
                           sizeof(struct sockaddr_ll)) < 0) {
                        Warning("Failed to send packet");
		    }
		}
	    }
        }
    }
}

void
Transport::SetSocketDest(struct sockaddr_ll *sll, specpaxos::ReplicaAddress *replica) {
    sll->sll_ifindex = ifopts.ifr_ifindex;
    sll->sll_halen = ETH_ALEN;
    // sll_addr has length 8, mac addrs only 6 long
    sll->sll_addr[0] = replica->mac[0];
    sll->sll_addr[1] = replica->mac[1];
    sll->sll_addr[2] = replica->mac[2];
    sll->sll_addr[3] = replica->mac[3];
    sll->sll_addr[4] = replica->mac[4];
    sll->sll_addr[5] = replica->mac[5];
}

void
Transport::SetPacketDest(uint8_t *packet, specpaxos::ReplicaAddress *replica) {
    struct iphdr *iph;
    struct udphdr *udph;
    struct ether_header *eh;
    eh = (struct ether_header *)packet;
    iph = (struct iphdr *)(packet + sizeof(struct ether_header));
    udph = (struct udphdr *)(packet + sizeof(struct ether_header) + sizeof(struct iphdr));

    // Set ethernet src and dest.
    eh->ether_dhost[0] = replica->mac[0];
    eh->ether_dhost[1] = replica->mac[1];
    eh->ether_dhost[2] = replica->mac[2];
    eh->ether_dhost[3] = replica->mac[3];
    eh->ether_dhost[4] = replica->mac[4];
    eh->ether_dhost[5] = replica->mac[5];
    
    // Set IP destination based on replica address.
    uint32_t ip_dst;
    if (!inet_pton(AF_INET, replica->host.c_str(), &ip_dst)) {
	    Panic("Failed to parse replica IP address %s", replica->host.c_str());
    }
    iph->daddr = ip_dst;

    // Set UDP destination port based on replica port.
    udph->dest = htons(stoi(replica->port)); 

    // Recompute checksum for IP (UDP checksum disabled).
    iph->check = 0;
    iph->check = cksum((unsigned short *)iph, sizeof(struct iphdr));
    udph->check = 0;
    udph->check = udp_checksum(udph, ntohs(udph->len), iph->saddr, iph->daddr);
}

void
Transport::SetOuterPacketDestSrc(uint8_t *packet, specpaxos::ReplicaAddress *replica) {
    struct iphdr *iph;
    struct udphdr *udph;
    struct ether_header *eh;
    eh = (struct ether_header *)packet;
    iph = (struct iphdr *)(packet + sizeof(struct ether_header));
    udph = (struct udphdr *)(packet + sizeof(struct ether_header) + sizeof(struct iphdr));

    // Set ethernet src and dest.
    eh->ether_dhost[0] = replica->mac[0];
    eh->ether_dhost[1] = replica->mac[1];
    eh->ether_dhost[2] = replica->mac[2];
    eh->ether_dhost[3] = replica->mac[3];
    eh->ether_dhost[4] = replica->mac[4];
    eh->ether_dhost[5] = replica->mac[5];
    const specpaxos::ReplicaAddress *sequencer = global_config->sequencer();
    eh->ether_shost[0] = sequencer->mac[0];
    eh->ether_shost[1] = sequencer->mac[1];
    eh->ether_shost[2] = sequencer->mac[2];
    eh->ether_shost[3] = sequencer->mac[3];
    eh->ether_shost[4] = sequencer->mac[4];
    eh->ether_shost[5] = sequencer->mac[5];

    // Set IP destination based on replica address.
    uint32_t ip_dst;
    if (!inet_pton(AF_INET, replica->host.c_str(), &ip_dst)) {
	    Panic("Failed to parse replica IP address %s", replica->host.c_str());
    }
    uint32_t ip_src;
    if (!inet_pton(AF_INET, sequencer->host.c_str(), &ip_src)) {
	    Panic("Failed to parse sequencer IP address %s", sequencer->host.c_str());
    }
    iph->daddr = ip_dst;
    iph->saddr = ip_src;
    iph->tot_len = htons(ntohs(iph->tot_len) + sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr));

    // Set UDP destination port based on replica port.
    udph->dest = htons(stoi(replica->port)); 
    udph->len = htons(ntohs(udph->len) + sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr));

    // Recompute checksum for IP (UDP checksum disabled).
    iph->check = 0;
    iph->check = cksum((unsigned short *)iph, sizeof(struct iphdr));
    udph->check = 0;
    udph->check = udp_checksum(udph, ntohs(udph->len), iph->saddr, iph->daddr);
}

// From https://github.com/rbaron/raw_tcp_socket/blob/master/raw_tcp_socket.c.
unsigned short 
Transport::cksum(unsigned short *ptr,int nbytes) {
   long sum = 0;  /* assume 32 bit long, 16 bit short */

   while(nbytes > 1){
       sum += *(ptr)++;
       if(sum & 0x80000000)   /* if high order bit set, fold */
           sum = (sum & 0xFFFF) + (sum >> 16);
       nbytes -= 2;
   }

   if(nbytes)       /* take care of left over byte */
       sum += (unsigned short) *(unsigned char *)ptr;
          
   while(sum>>16)
       sum = (sum & 0xFFFF) + (sum >> 16);

   return ~sum;
}

uint16_t 
Transport::udp_checksum(const void *buff, size_t len, uint32_t src_addr, uint32_t dest_addr)
{
        const uint16_t *buf = (const uint16_t *)buff;
        uint16_t *ip_src=(uint16_t *)&src_addr, *ip_dst=(uint16_t *)&dest_addr;
        uint32_t sum;
        size_t length=len;

        // Calculate the sum                                            //
        sum = 0;
        while (len > 1)
        {
                sum += *buf++;
                if (sum & 0x80000000)
                        sum = (sum & 0xFFFF) + (sum >> 16);
                len -= 2;
        }

        if ( len & 1 )
                // Add the padding if the packet lenght is odd          //
                sum += *((uint8_t *)buf);

        // Add the pseudo-header                                        //
        sum += *(ip_src++);
        sum += *ip_src;

        sum += *(ip_dst++);
        sum += *ip_dst;

         sum += htons(IPPROTO_UDP);
         sum += htons(length);

         // Add the carries                                              //
         while (sum >> 16)
                 sum = (sum & 0xFFFF) + (sum >> 16);

         // Return the one's complement of sum                           //
         return ( (uint16_t)(~sum)  );
}

bool
Transport::ProcessPacket(uint8_t *packet, size_t len)
{
    //struct ether_header *eh;
    struct iphdr *iph;
    struct udphdr *udph;
    struct sockaddr_storage saddr;
    uint8_t *datagram, ngroups;
    char destip[INET6_ADDRSTRLEN];
    uint16_t group_bitmap;

    if (len < sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct udphdr)) {
        return false;
    }

    //eh = (struct ether_header *)packet;
    iph = (struct iphdr *)(packet
                           + sizeof(struct ether_header));
    udph = (struct udphdr *)(packet
                             + sizeof(struct ether_header)
                             + sizeof(struct iphdr));
    datagram = (uint8_t *)(packet
                           + sizeof(struct ether_header)
                           + sizeof(struct iphdr)
                           + sizeof(struct udphdr));

    /* All network ordered messages are multicast.
     * Check ethernet destination is FF:FF:FF:FF:FF:FF,
     * and IP destination is the group multicast address.
     */
    //for (int i = 0; i < ETH_ALEN; i++) {
        // Remove checks for multicast
	//if (eh->ether_dhost[i] != 0xFF) {
        //    return false;
        //}
    //}

    ((struct sockaddr_in *)&saddr)->sin_addr.s_addr = iph->daddr;
    inet_ntop(AF_INET, &((struct sockaddr_in *)&saddr)->sin_addr, destip, sizeof(destip));

    //if (strcmp(destip, this->config->GetGroupAddr().c_str())) {
    //    return false;
    //}

    /* Network ordered packet header format:
     * FRAG_MAGIC(32) | header data len (32) | original udp src (16) |
     * session ID (64) | number of groups (32) |
     * group1 ID (32) | group1 sequence number (64) |
     * group2 ID (32) | group2 sequence number (64) |
     * ...
     */

    if (*(uint32_t *)datagram != NONFRAG_MAGIC) {
        // Only sequence the packet if it is not
        // fragmented.
        return false;
    }

    datagram += sizeof(uint32_t) + sizeof(uint32_t); // now points to udp src
    /* Write the original udp src into header */
    *(uint16_t *)datagram = udph->source;

    datagram += sizeof(uint16_t); // now points to session ID
    *(uint64_t *)datagram = this->sequencer->GetSequencerID();

    datagram += sizeof(uint64_t); // now points to number of groups
    ngroups = *(uint32_t *)datagram;

    datagram += sizeof(uint32_t); // now points to group1 ID
    group_bitmap = 0;
    for (int i = 0; i < ngroups; i++) {
        uint32_t groupid = *(uint32_t *)datagram;
        datagram += sizeof(uint32_t);
        *(uint64_t *)datagram = this->sequencer->Increment(groupid);
        datagram += sizeof(uint64_t);
        group_bitmap |= (1 << groupid);
    }

    /* Update udp header src field with the group bitmap.
     * Switches use this bitmap to perform group cast.
     */
//    udph->source = htons(group_bitmap);
    udph->check = 0; // disable udp checksum
    return true;
}

} // namespace sequencer

int main(int argc, char *argv[]) {
    const char *config_path = nullptr;
    const char *global_config_path = nullptr;
    int opt;

    while ((opt = getopt(argc, argv, "c:C:")) != -1) {
        switch (opt) {
        case 'c':
            config_path = optarg;
            break;

	case 'C':
	    global_config_path = optarg;
	    break;

        default:
            fprintf(stderr, "Unknown argument %s\n", argv[optind]);
            break;
        }
    }

    if (config_path == nullptr) {
        fprintf(stderr, "option -c is required\n");
        return 1;
    }

    if (global_config_path == nullptr) {
        fprintf(stderr, "option -C is required\n");
	return 1;
    }

    ifstream config_stream(config_path);
    if (config_stream.fail()) {
        fprintf(stderr, "unable to read configuration file: %s\n",
                config_path);
        return 1;
    }

    ifstream global_config_stream(global_config_path);
    if (global_config_stream.fail()) {
        fprintf(stderr, "unable to read global configuration file: %s\n",
                global_config_path);
        return 1;
    }


    specpaxos::Configuration global_config(global_config_stream);
    sequencer::Configuration config(config_stream);
    sequencer::Sequencer sequencer(0);
    sequencer::Transport transport(&sequencer, &config, &global_config);
    transport.Run();

    return 0;
}
