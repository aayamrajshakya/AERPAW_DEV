cd ~/containerization

mkdir -p logs

# Start nrf
(cd logs && touch nrf.log udr.log udm.log ausf.log amf.log smf.log upf.log mysql.log)

# Start asterisk ims
nohup asterisk -fp &

# Start oai ext dn (you need to fill this in)

# Start mysql
nohup service mysql start > ./logs/mysql.log &
sleep 5

# Start nrf
nohup ./nrf/build/nrf/build/nrf -c ./nrf/etc/config.yaml -o > ./logs/nrf.log &
sleep 5

# Start udr
nohup ./udr/build/udr/build/udr -c ./udr/etc/config.yaml -o > ./logs/udr.log &
sleep 5

# Start udm
nohup ./udm/build/udm/build/udm -c ./udm/etc/config.yaml -o > ./logs/udm.log &
sleep 5

# Start ausf
nohup ./ausf/build/ausf/build/ausf -c ./ausf/etc/config.yaml -o > ./logs/ausf.log &
sleep 5

# Start amf
nohup ./amf/build/amf/build/amf -c ./amf/etc/config.yaml -o > ./logs/amf.log &
sleep 5

# Start smf
nohup ./smf/build/smf/build/smf -c ./smf/etc/config.yaml -o > ./logs/smf.log &
sleep 5

# Start upf
nohup ./upf/build/upf/build/upf -c ./upf/etc/config.yaml -o > ./logs/upf.log &
sleep 5

# Make start_trf.sh executable
chmod +x /usr/local/bin/start_trf.sh

# Run start_trf.sh in the background
nohup /usr/local/bin/start_trf.sh > /dev/null 2>&1 &

# Return to the original directory
cd -
