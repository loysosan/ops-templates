provider "ansible" {}

resource "ansible_host" "server1" {
  name     = "192.168.1.104"
  groups   = ["servers"]
  variables = {
    ansible_user                 = "kas"
    ansible_ssh_private_key_file = "~/.ssh/id_rsa"
  }
}

resource "ansible_host" "server2" {
  name     = "192.168.1.106"
  groups   = ["servers"]
  variables = {
    ansible_user                 = "kas"
    ansible_ssh_private_key_file = "~/.ssh/id_rsa"
  }
}


resource "ansible_group" "servers" {
  name     = "somegroup"
  children = ["server1", "server2"]
  variables = {
    hello = "from group!"
  }
}

resource "ansible_playbook" "create_file" {
  name       = "create_file"
  playbook   = "${path.module}/ansible/create-file.yaml"
  groups     = ["servers"]
  replayable = true
  verbosity  = 4
}