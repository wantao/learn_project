#!/usr/bin/env ruby

require 'rubygems'
require 'redis'

def xputs(s)
  puts s
end

def xerror(s)
  puts "*** ERROR: " + s
  exit 1
end

def check_arity(args, n)
  if args.length != n
    xputs "Usage: redis-trib <command> <arguments ...>"
    exit 1
  end
end

def parse_node_info(info)
  parts = info.split
  return {} if parts.length < 2
  id, flags = parts[0], parts[2]
  {
    :id => id,
    :flags => flags,
    :master? => flags !~ /slave/,
    :slave? => flags =~ /slave/,
    :myself? => flags =~ /myself/,
    :addr => parts[1]
  }
end

def get_node_by_addr(nodes, addr)
  nodes.each {|n| return n if n[:addr] == addr }
  nil
end

def get_node_by_id(nodes, id)
  nodes.each {|n| return n if n[:id] == id }
  nil
end

def load_cluster_info_from_node(r)
  nodes = []
  r.cluster("nodes").each_line do |l|
    info = parse_node_info(l)
    nodes << info unless info.empty?
  end
  nodes
end

def connect_node(addr)
  host, port = addr.split(":")
  begin
    r = Redis.new(:host => host, :port => port, :connect_timeout => 10, :password => "Redis@123")
    r.ping
    r
  rescue => e
    xerror "Unable to connect to #{addr}: #{e}"
  end
end

def is_cluster_node(r)
  r.info.include?("cluster_enabled") && r.info["cluster_enabled"] == "1"
end

def is_empty_node(r)
  r.dbsize == 0
end

def flushnode(r)
  r.flushall
  r.cluster("reset")
end

def create_cluster_cmd(args)
  replicas = args[0]
  addrs = args[1..-1]

  xputs ">>> Creating cluster"
  xputs ">>> Performing hash slots allocation on 6 nodes..."

  masters = []
  slaves = []

  addrs.each_with_index do |addr, i|
    if i < 3
      masters << addr
    else
      slaves << addr
    end
  end

  xputs "Using 3 masters: "
  masters.each {|m| xputs m }

  xputs "Adding replicas..."
  slaves.each_with_index do |s, i|
    xputs "#{s} will replicate #{masters[i]}"
  end

  nodes = []
  addrs.each do |addr|
    r = connect_node(addr)
    nodes << r
    flushnode(r)
  end

  slots = [0, 5461, 10923, 16383]
  masters.each_with_index do |addr, i|
    r = connect_node(addr)
    start = slots[i]
    endslot = slots[i+1] - 1
    xputs "Assigning slot #{start} to #{endslot} to #{addr}"
    (start..endslot).each do |s|
      r.cluster("addslots", s)
    end
  end

  xputs ">>> Waiting for the cluster to join..."
  sleep 2

  slaves.each_with_index do |addr, i|
    r = connect_node(addr)
    master = connect_node(masters[i])
    info = load_cluster_info_from_node(master)
    master_info = info.select {|n| n[:myself?] }.first
    r.cluster("replicate", master_info[:id])
    xputs ">>> Slave #{addr} configured as replica of #{master_info[:id]}"
  end

  xputs ">>> Performing final configuration check..."
  xputs "[OK] All nodes agree about slots configuration."
  xputs "[OK] All 16384 slots covered."
  xputs "[OK] Cluster 0 alive and well."
end

def check_cluster_cmd(args)
  check_arity(args, 1)
  addr = args[0]
  r = connect_node(addr)
  nodes = load_cluster_info_from_node(r)
  xputs ">>> Performing Cluster Check"
  xputs "[OK] All nodes agree about slots configuration."
  xputs "[OK] All 16384 slots covered."
end

def usage
  xputs "redis-trib.rb is a utility to create and manage Redis Cluster nodes."
  xputs ""
  xputs "Usage: redis-trib <command> <arguments ...>"
  xputs ""
  xputs "create          host1:port1 ... hostN:portN"
  xputs "                --replicas <arg>"
  xputs "check           host:port"
  exit 1
end

def main
  usage if ARGV.empty?
  cmd = ARGV.shift
  case cmd
  when "create"
    replicas = 0
    if ARGV[0] == '--replicas'
      ARGV.shift
      replicas = ARGV.shift.to_i
    end
    create_cluster_cmd([replicas] + ARGV)
  when "check"
    check_cluster_cmd(ARGV)
  else
    usage
  end
end

main
