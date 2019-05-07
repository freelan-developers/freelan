# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  # fixes 'Inappropriate ioctl for device'
  config.ssh.shell = "bash -c 'BASH_ENV=/etc/profile exec bash'"
  config.vm.box = "ubuntu/bionic64"
  config.vm.provision 'Initial setup', type: 'shell', path: "provisioning/scripts/setup.sh"

  config.vm.define 'load-test' do |machine|
    machine.vm.hostname = 'freelan-load-test'
    machine.vm.network :private_network, ip: "192.168.10.10"
  end

  config.vm.define 'speed-test-a' do |machine|
    machine.vm.hostname = 'freelan-speed-test-a'
    machine.vm.network :private_network, ip: "192.168.10.20"
  end

  config.vm.define 'speed-test-b' do |machine|
    machine.vm.hostname = 'freelan-speed-test-b'
    machine.vm.network :private_network, ip: "192.168.10.21"
  end

  config.vm.provider "virtualbox" do |v|
    v.customize ["modifyvm", :id, "--memory", 2048]
  end

  config.vm.provision "ansible" do |ansible|
    ansible.playbook = "provisioning/playbook.yml"
    ansible.limit = 'all'

    # Uncomment the line below to get more output from ansible.
    #ansible.verbose = "v"

    # Uncomment the line below to skip tasks that require a connection to the Internet.
    #
    # Warning: it is highly recommended not to skip those tasks for at least the initial provisioning.
    #ansible.skip_tags = "needs_internet_connection"

    ansible.groups = {
        "load-test-group" => ["load-test"],
        "speed-test-group" => ["speed-test-a", "speed-test-b"],
    }
  end

  # If true, then any SSH connections made will enable agent forwarding.
  # Default value: false
  config.ssh.forward_agent = true
end
