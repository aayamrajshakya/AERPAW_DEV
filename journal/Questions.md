# Open Questions:

- Why does the [upf](upf/etc/config.yaml) service not work? It seems that the
  n4, n3, etc ips are wrong.
- Why do IP addresses not bind to the ones in [etc/hosts](hosts)? (right now
  they are binding to IPv6 addresses)
- How can we configure the gNB AMF and UPF IP addresses such that they will
  connect to the core network. There must be a configuration file somewhere that
  is editable before running the gNB.
