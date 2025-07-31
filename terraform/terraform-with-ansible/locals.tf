locals {
  sbox_vms = [
    {
      name = "TEST-PP-HOST-01"
      networks = [{ ip = "192.168.1.104" }]
    },
    {
      name = "TEST-PP-HOST-02"
      networks = [{ ip = "192.168.1.106" }]
    }
  ]
}
