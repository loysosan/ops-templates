# Виконання playbook для всіх хостів через один ресурс
resource "ansible_playbook" "install_tcpdump" {
  for_each = { for vm in local.sbox_vms : vm.name => vm }
  
  playbook = "${path.module}/ansible-playbooks/install-tcpdump.yml"
  
  # Inventory configuration - кожен хост окремо
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
  }
  
  check_mode = false
  diff_mode  = false
  verbosity  = 3
  replayable = true
}

# Output для налагодження всіх хостів
output "ansible_outputs" {
  value = {
    for name, playbook in ansible_playbook.install_tcpdump : name => {
      stdout = playbook.ansible_playbook_stdout
      stderr = playbook.ansible_playbook_stderr
    }
  }
}k