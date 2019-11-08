#!/usr/bin/ruby

$:.unshift File.dirname(__FILE__)

# require 'ripl'
# require 'exprlib'
def read_pa(pafn)
    pa=File.readlines(pafn).collect{|x|x.strip}
    header_end = -1
    (1..10).each{|i| header_end =i if /\----+/.match(pa[i])}
    return nil if header_end == -1
    labels = pa[(header_end+1)..-1].collect{|x|x.to_i}
    k = pa[1].to_i
    return [labels,k]
end



def show_cluster_samples(clusters)
    k=clusters.size
    #clu_str =""
    for i in 0..(k-1)
        #puts "#{i}:"+clusters[i].sample(10).inspect
        items = clusters[i].sample(20).collect{|x|"<li>#{x}</li>"}.join(" ")
        #puts "#{i} #{clusters[i].size} "+clusters[i].sample(10).inspect
        puts "<hr/><b>Cluster #{i+1} (size=#{clusters[i].size}</b>, 20 samples) <ul>#{items}</ul>"
    end
end

dsfn=ARGV[0]
pafn=ARGV[1]

lines=File.readlines(dsfn).collect{|x|x.strip}
#puts lines[0..2]
#puts pa[0..5]
(labels,k)=read_pa(pafn)
clusters=[]

#Ripl.start :binding => binding
#k=12
#labels=label_str.split().collect{|x|x.to_i}
#lines=File.readlines("data/countries_sub_K12_n50_tp0100.txt").collect{|x|x.strip}

#Ripl.start :binding => binding
for i in 1..k
    clusters[i-1] = []
end

for j in 0..(lines.size-1)
    clusters[labels[j]-1] << lines[j]
end

puts "<html><head></head><body>"
show_cluster_samples(clusters)
puts "</body></html>"


#Ripl.start :binding => binding
