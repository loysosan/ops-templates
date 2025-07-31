resource "ansible_playbook" "install_tcpdump" {
  for_each = { for vm in local.sbox_vms : vm.name => vm }
  
  playbook = "${path.module}/ansible/ansible.yaml"
  
  # Inventory configuration
  name   = each.value.name
  groups = ["test_servers"]


  
  # Connection configuration and other vars
  extra_vars = {
    ansible_host                 = each.value.networks[0].ip
    ansible_user                 = var.vm_user
    ansible_ssh_private_key_file = var.ssh_private_key_path
    ansible_ssh_common_args      = "-o StrictHostKeyChecking=no"
    ansible_python_interpreter   = "/usr/bin/python3"
    ansible_connection           = "ssh"
    env                          = "sbox"
    tags                         = "configure"
  }
  
  check_mode = false
  diff_mode  = false
  verbosity  = 3
  replayable = true
}

output "ansible_outputs" {
  value = {
    for name, playbook in ansible_playbook.install_tcpdump : name => {
     # stdout = playbook.ansible_playbook_stdout
      stderr = playbook.ansible_playbook_stderr
    }
  }
}