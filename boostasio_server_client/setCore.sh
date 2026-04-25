mkdir -p /corefile
chmod 777 /corefile
echo "/corefile/core-%e-%p-%t" >> /proc/sys/kernel/core_pattern
echo "kernel.core_pattern=/corefile/core-%e-%p-%t" >> /etc/sysctl.conf
sysctl -p
ulimit -c unlimited
echo "* soft core unlimited" >> /etc/security/limits.conf

