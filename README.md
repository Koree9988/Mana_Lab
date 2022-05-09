"# MANA_Lab" 
The esp32 sim800l can use only 2G sim
each service don't have the same apn

for True:
  apn[]      = "internet"; 
  gprsUser[] = "true"; 
  gprsPass[] = "true";
for Ais:
  apn[]      = "internet"; 
  gprsUser[] = "ais"; 
  gprsPass[] = "ais";
for Dtac:
  apn[]      = "www.dtac.co.th"; 
  gprsUser[] = ""; 
  gprsPass[] = "";
  
Reference:http://itnews4u.com/What-is-APN-and-How-to-setting-APN.html
