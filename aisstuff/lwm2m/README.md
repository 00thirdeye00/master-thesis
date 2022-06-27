## LWM2M
start leshan server
cp lwm2m_firmware.[c|h] to contiki-ng/os/serverices/lwm2m
## cooja
start cooja and open lwm2m.csc
sudo <path to >/tunslip6 -a 127.0.0.1 fd00::1/64
##
Once devices are registered download can be initiated by updating package uri in web gui. 
Can be tested by setting the the uri to "http://[fd00::201:1:1:1]/". That will download from the web server located in the border router. If simplecontentserver is used "http://[fd00::1]:8081/" can be used




