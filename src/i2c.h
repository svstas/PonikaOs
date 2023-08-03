bool find_i2c(uint16_t addr) {
for (std::vector<uint16_t>::const_iterator i = conf.i2c.begin(); i != conf.i2c.end(); ++i) if (addr == *i) return true;
return false;
}


void scan_i2c(bool out = true) { 
   
  byte error, address;
  int nDevices;
  String xout = "";

  std::vector<uint16_t>i2c;
 
  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
 
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
//    I2Cone.beginTransmission(address);
//    error = I2Cone.endTransmission();
    if (error == 0)
    {
//      xout+="I2C device found at address 0x";
      xout+="0x";
      if (address<16) 

        xout+=("0");
        xout+=String(address,HEX);        
        i2c.push_back(address);
        xout+="\n";
      nDevices++;
    }
    else if (error==4) 
    {
      xout+="Unknow error at address 0x";
      if (address<16) 
        xout+="0";
        xout+=String(address,HEX);
        xout+="\n";
    }    
  }
  if (nDevices == 0)
    xout+="No I2C devices found\n";
  else
    xout+="";
    if (out) sendEvent(xout);
// return xout;
i2ctouch=true;

  //std::vector<uint16_t>(conf.i2c).swap(i2c);
  conf.i2c = i2c;
//  BMESensor.refresh();
  std::vector<uint16_t>(i2c).swap(i2c);
}
