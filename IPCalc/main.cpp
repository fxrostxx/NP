#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstdint>
#include <bitset>
#include <string>
using namespace std;

uint32_t dec_dot_to_uint32(string addr)
{
	char* addr_c = new char[addr.size() + 1];
	strcpy(addr_c, addr.c_str());
	const char* delimeter = ".";
	uint8_t octets[4]{};
	char* token;
	int i = 0;
	for (token = strtok(addr_c, delimeter); token != nullptr && i < 4; token = strtok(nullptr, delimeter))
		octets[i++] = atoi(token);
	delete[] addr_c;
	return (octets[0] << 24) | (octets[1] << 16) | (octets[2] << 8) | octets[3];
}
string uint32_to_dec_dot(uint32_t addr)
{
	uint8_t octets[4];
	octets[0] = (addr >> 24) & 0xFFU;
	octets[1] = (addr >> 16) & 0xFFU;
	octets[2] = (addr >> 8) & 0xFFU;
	octets[3] = addr & 0xFFU;
	string addr_dec_dot;
	for (int i = 0; i < 4; ++i)
	{
		addr_dec_dot.append(to_string(octets[i]));
		if (i != 3) addr_dec_dot.append(".");
	}
	return addr_dec_dot;
}
string uint32_to_bin_dot(uint32_t addr)
{
	string addr_bin_dot = bitset<32>(addr).to_string();
	for (int i = 24; i > 0; i -= 8) addr_bin_dot.insert(i, ".");
	return addr_bin_dot;
}
uint32_t prefix_to_mask(uint8_t prefix)
{
	return prefix ? 0xFFFFFFFFU << (32 - prefix) : 0;
}
uint32_t get_network_addr(uint32_t ip_addr, uint32_t mask)
{
	return ip_addr & mask;
}
uint32_t get_broadcast_addr(uint32_t ip_addr, uint32_t mask)
{
	return ip_addr | ~mask;
}
uint32_t get_min_host_addr(uint32_t ip_addr, uint32_t mask)
{
	if (mask == 0xFFFFFFFF || mask == 0xFFFFFFFE) return 0;
	return get_network_addr(ip_addr, mask) + 1U;
}
uint32_t get_max_host_addr(uint32_t ip_addr, uint32_t mask)
{
	if (mask == 0xFFFFFFFF || mask == 0xFFFFFFFE) return 0;
	return get_broadcast_addr(ip_addr, mask) - 1U;
}

int main()
{
	string ip_addr;
	uint16_t prefix;
	while (true)
	{
		cout << "IP address: "; cin >> ip_addr;
		cout << "Prefix: "; cin >> prefix;

		cout << endl;

		uint32_t ip = dec_dot_to_uint32(ip_addr);
		uint32_t mask = prefix_to_mask(prefix);
		uint32_t network_ip = get_network_addr(ip, mask);
		uint32_t broadcast_ip = get_broadcast_addr(ip, mask);
		uint32_t min_host_ip = get_min_host_addr(ip, mask);
		uint32_t max_host_ip = get_max_host_addr(ip, mask);

		cout << uint32_to_dec_dot(ip) << '\t' << uint32_to_bin_dot(ip) << endl;
		cout << uint32_to_dec_dot(mask) << '\t' << uint32_to_bin_dot(mask) << endl;

		cout << endl;

		cout << "Network address:\t" << uint32_to_dec_dot(network_ip) << '\t' << uint32_to_bin_dot(network_ip) << endl;
		cout << endl;
		cout << "Broadcast address:\t" << uint32_to_dec_dot(broadcast_ip) << '\t' << uint32_to_bin_dot(broadcast_ip) << endl;
		cout << endl;
		if (mask == 0xFFFFFFFF || mask == 0xFFFFFFFE) cout << "No host addresses" << endl;
		else
		{
			cout << "Range:" << endl;
			cout << uint32_to_dec_dot(min_host_ip) << " - " << uint32_to_dec_dot(max_host_ip) << endl;
			cout << uint32_to_bin_dot(min_host_ip) << " - " << uint32_to_bin_dot(max_host_ip) << endl;
		}		
		cout << endl;
	}
	return 0;
}