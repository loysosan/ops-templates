locals {
  sbox_vms = [
    {
      name = "TEST-HOST-01"
      networks = [{ ip = "192.168.1.104" }]
    },
    {
      name = "TEST-HOST-02"
      networks = [{ ip = "192.168.1.106" }]
    }
  ]
}
