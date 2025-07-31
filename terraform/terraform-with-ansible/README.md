# Terraform + Ansible Provider Example

This example demonstrates how to use the [Terraform Ansible Provider](https://github.com/ansible/terraform-provider-ansible) to automate running Ansible playbooks on remote servers.

## How it works

- **Terraform** manages infrastructure and can run Ansible playbooks directly via the `ansible/ansible` provider.
- You define hosts (the `ansible_host` resource), groups, and the playbook (`ansible_playbook`) in `.tf` files.
- The provider automatically generates an Ansible inventory based on your resources.
- When you run `terraform apply`:
  - Terraform connects to servers via SSH (using Ansible),
  - Executes the specified playbook with the required variables and parameters.

## Main resources

- `ansible_host` — describes an individual server (IP, user, key, group, etc).
- `ansible_playbook` — describes running a playbook for a group of hosts or a single host.

## Example usage

```hcl
provider "ansible" {}

resource "ansible_host" "server1" {
  name     = "192.168.1.104"
  groups   = ["servers"]
  variables = {
    ansible_user                 = "kas"
    ansible_ssh_private_key_file = "~/.ssh/id_rsa"
  }
}

resource "ansible_playbook" "create_file" {
  name       = "create_file"
  playbook   = "${path.module}/ansible/create-file.yaml"
  groups     = ["servers"]
  replayable = true
}
```

## Usage

1. Install Terraform and Ansible.
2. Initialize Terraform:
   ```sh
   terraform init
   ```
3. Apply the configuration:
   ```sh
   terraform apply
   ```

## Notes
- The provider is not published in the official Terraform Registry, so you need a local binary (see the provider documentation).
- The Ansible inventory is generated automatically from `ansible_host` and `groups` resources.
- All connection variables (user, key, etc) are set via the `variables` block in the `ansible_host` resource.

---

More info: [Ansible Terraform Provider Docs](https://github.com/ansible/terraform-provider-ansible)
