## Router2 — основной выход в интернет через ether1 (к Router1),
## дублирующий выход через ether2 (к основному роутеру), мост ether3-ether4 с DHCP-сервером.
## Этими командами настроен Router2 (вставлены в терминал RouterOS).

/ip dhcp-client add interface=ether1 add-default-route=yes default-route-distance=1 use-peer-dns=no disabled=no
/ip dhcp-client add interface=ether2 add-default-route=yes default-route-distance=2 use-peer-dns=no disabled=no

/interface bridge add name=bridge-lan
/interface bridge port add bridge=bridge-lan interface=ether3
/interface bridge port add bridge=bridge-lan interface=ether4

/ip address add address=192.168.20.1/24 interface=bridge-lan

/ip pool add name=pool-lan2 ranges=192.168.20.10-192.168.20.200
/ip dhcp-server add name=dhcp-lan2 interface=bridge-lan address-pool=pool-lan2 lease-time=1d disabled=no
/ip dhcp-server network add address=192.168.20.0/24 gateway=192.168.20.1 dns-server=8.8.8.8,1.1.1.1

/ip firewall nat add chain=srcnat out-interface=ether1 action=masquerade
/ip firewall nat add chain=srcnat out-interface=ether2 action=masquerade
