Vagrant.configure("2") do |config|
  config.vm.box = "generic/ubuntu2204"

  # QEMU works better with rsync than native shared folders
  config.vm.synced_folder ".", "/workspace/imgcrypt_bench", type: "rsync"

  config.vm.provider "qemu" do |qe|
    qe.memory = "4096"
    qe.cpus = 2
  end

  config.vm.boot_timeout = 600

  config.vm.provision "shell", inline: <<-SHELL
    set -e
    export DEBIAN_FRONTEND=noninteractive

    # apt-get update

    # apt-get install -y \
    #   build-essential \
    #   cmake \
    #   ninja-build \
    #   pkg-config \
    #   git \
    #   curl \
    #   wget \
    #   libssl-dev \
    #   libopencv-dev \
    #   libsodium-dev \
    #   libpapi-dev \
    #   libgsl-dev

    mkdir -p /workspace/imgcrypt_bench

    # echo "==== Versions ===="
    # openssl version || true
    # pkg-config --modversion openssl || true
    # pkg-config --modversion opencv4 || true
    # pkg-config --modversion libsodium || true
    # pkg-config --modversion gsl || true
    # papi_avail || true
  SHELL
end