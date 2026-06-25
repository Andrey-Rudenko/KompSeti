## Router1 — выход в интернет через ether1, мост ether2-ether4 с DHCP-сервером.
## Этими командами настроен Router1 (вставлены в терминал RouterOS).

/interface bridge add name=bridge-lan

/interface bridge port add bridge=bridge-lan interface=ether2
/interface bridge port add bridge=bridge-lan interface=ether3
/interface bridge port add bridge=bridge-lan interface=ether4

/ip dhcp-client add interface=ether1 disabled=no

/ip address add address=192.168.10.1/24 interface=bridge-lan

/ip pool add name=pool-lan ranges=192.168.10.10-192.168.10.200

/ip dhcp-server add name=dhcp-lan interface=bridge-lan address-pool=pool-lan lease-time=1d disabled=no
/ip dhcp-server network add address=192.168.10.0/24 gateway=192.168.10.1 dns-server=8.8.8.8,1.1.1.1

/ip firewall nat add chain=srcnat out-interface=ether1 action=masquerade
