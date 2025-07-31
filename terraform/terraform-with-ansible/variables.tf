variable "vm_user" {
  description = "Username for VM access"
  type        = string
  default     = "kas"
}

variable "ssh_private_key_path" {
  description = "Path to SSH private key"
  type        = string
  default     = "~/.ssh/id_rsa"
}
