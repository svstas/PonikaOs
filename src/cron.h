




/* 
class Cron {

  private:

    struct Func {
      uint8_t pin;
      uint8_t xid = 0;
      //  Bool f;
      uint8_t sensor;
      uint16_t offtime;
      //  bool tchk = false;
      bool more = false;
      char hms[5];
      char val[4];

    };

    struct Lcron {
      uint8_t xpcf;
      uint8_t pin;
      uint32_t tstart;
      uint32_t tend;
    };

    std::vector<Lcron> vc;

    //Lcron vc[16] = {};
    int ptr = 0;

    std::vector<Func> Xfunc;

    void setTask(String sx, uint8_t pin, uint8_t rtime, uint8_t xid) {

      //  char task[10];
      //  sx.toCharArray(task,10);
      bool more = true;

      String sens = "";
      String val = "";

      uint8_t sep = sx.indexOf(">");
      if (sep && sep < sx.length()) {
        sens = sx.substring(0, sep);
        val = sx.substring(sep + 1);
      } else {
        sep = sx.indexOf("<");
        if (sep && sep < sx.length()) {
          more = false;
          sens = sx.substring(0, sep);
          val = sx.substring(sep + 1);
        }
      }

      uint8_t sensor = mysensors->getSensId(&sens[0]);
      String par = "";
      if (sensor == 100) {
        sep = sens.indexOf(".");
        if (sep > 0 && sep < sens.length()) par = sens.substring(sep + 1);
      } else mysensors->LSensors[sensor].relays.push_back(pin);

      Func nf;
      nf.pin = pin;
      nf.xid = xid;
      nf.sensor = sensor;
      nf.offtime = rtime;
      strcpy(nf.val, val.c_str());
      nf.more = more;
      strcpy(nf.hms, par.c_str());
      Xfunc.push_back(nf);

      sens = String(); sx = String(); par = String();
    }

    void sort() {
      std::sort(vc.begin(), vc.end(), [] (Lcron const & a, Lcron const & b) {
        return a.tstart < b.tstart;
      });
    }


  public:

    void xsaveState() {
      File f;
      statex = pcf1.read8();
      f = SPIFFS.open("/admin/ini/state.ini", "w");
      if (!f) {
        Serial.println("Failed to open state.ini");
      }
      f.println("#" + String(statex) + ":" + now());
      for (uint8_t i = 0; i < ptr; i++) {
        f.println(String(vc[i].pin) + ":" + String(vc[i].tstart) + ":" + String(vc[i].tend));
      }
      f.close();
    };


    bool relay[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    bool orelay[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    //uint8_t irelay[8] = {1,1,1,1,1,1,1,1};
    //uint8_t irelay[8] = {0,0,0,0,0,0,0,0};
    uint8_t irelay[8] = {1, 1, 1, 1, 1, 1, 1, 1}; //инверсия (наоборот с rstat)
    bool rstat[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    //bool rstat[8] = {1,1,1,1,1,1,1,1};

    bool maintance = true;

    void loadRel() {
      ostate = statex;
      for (uint8_t i = 0; i < 8; i++) {
        relay[i] =  pcf1.read(conf.pcfr_swap[i]);
        orelay[i] = relay[i];
      }
    }

    uint8_t xloadState() {
      File f;
      uint8_t statexx;
      if (SPIFFS.exists("/admin/ini/state.ini")) f = SPIFFS.open("/admin/ini/state.ini", "r"); else return 0;
      if (!f) {
        Serial.println("Failed to open state.ini");
        return 0;
      }
      uint8_t cc = 0;
      while (f.available()) {
        if (cc > 20) continue; cc++;

        String xs = f.readStringUntil('\n');
        uint8_t sep;
        if (xs.substring(0, 1) == "#") {
          //    const char *ss;
          sep = xs.indexOf(":");
          if (!sep) {
            statexx = xs.substring(1).toInt();
          } else {
            statexx = xs.substring(1, sep).toInt();
            savedTime = xs.substring(sep + 1).toInt();
            if (savedTime > 0 && savedTime > conf.startTime) setTime(savedTime); // conf.startTime=now();

          }
          continue;
        }
        sep = xs.indexOf(":");
        uint8_t pin = xs.substring(0, sep).toInt();
        if (!sep) continue;
        uint8_t    sep1 = xs.indexOf(":", sep + 1);
        uint32_t tstart = xs.substring(sep + 1, sep1).toInt();
        uint16_t    tend = xs.substring(sep1 + 1).toInt();
        addCron(pin, tstart, tend, false, 1);
        xs = String();

      }
      if (ptr > 1)sort();
      f.close();
      return statexx;
    }


    void addCron(uint8_t pin, uint32_t tstart, uint32_t tend, bool save, uint8_t xpcf) {
      //if (xpcf==1) pin = conf.pcfr_swap[pin]; else pin = conf.pcfr_swap[pin];
      if (!tstart) return;
      vc.push_back({xpcf, pin, tstart, tend});
      ptr++;
      if (ptr > 1) sort();
      if (save) xsaveState();
    }

    void delC() {
      ptr--;
      vc.erase(vc.begin());
      vector<Lcron>(vc).swap(vc);
    }


    void sendR(bool xxxstart) {
      char buf[8] = {};

      for (int i = 0; i < 8; i++) {

        if (irelay[i] == 0) {
          if (orelay[i] != relay[i]) {
            if (relay[i] == rstat[i]) irelay[i] = 1;
            else irelay[i] = 2;
            orelay[i] = relay[i];
          }
        }


        const char * ron;
        const char * rof;
         
        if (rstat[i]) {
          
        ron = "-";
        rof = "+";

       
        } else {

        ron = "+";
        rof = "-";
       
        }
        if (relay[i] == 1) strcat(buf, rof); else strcat(buf, ron);
      }

      if (xxxstart) ostate = 0;
      if (statex != ostate) {
        charEventR(buf);
        ostate = statex;
      }
    }


    bool mycheck(uint8_t i) {

      String hms = Xfunc[i].hms;
      String val = Xfunc[i].val;
      uint8_t sensor = Xfunc[i].sensor;

      if (sensor == 100) return tcheck(val, hms, Xfunc[i].more);
      else {
        return scheck(sensor, val.toFloat(), Xfunc[i].more);
      }
      return false;
    }

    // ############################################### Перегруз
    void doCron() {

      bool   tmpr[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      int  startt[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      int    xoff[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      int  xoldid[8] = { -1, -1, -1, -1, -1, -1, -1, -1};
      bool save = false;
      bool rchange = false;
      bool isOn;
      std::vector<uint8_t> rcoff;

      if (ptr > 0) isOn = rstat[vc[0].pin];

      if (ptr > 0 && now() >= vc[0].tstart) {

        if (relay[vc[0].pin] != isOn && (vc[0].tend > 0)) {
          if (vc[0].xpcf == 1) {

            if (conf.pcfr_active) pcf1.write(conf.pcfr_swap[vc[0].pin], isOn);
            pix(vc[0].pin, isOn);
          } else {

            pcf2.write(conf.pcfs_swap[vc[0].pin], isOn);

          }

          relay[vc[0].pin] = isOn;
          rchange = true;
          //  sendEvent("ADD>>"+String(vc[0].tstart)+":"+String(now()+vc[0].tend),"sms");

          uint32_t tgo = vc[0].tend + now();
          uint8_t gpin = vc[0].pin;

          if (ptr > 0) delC();
          addCron(gpin, tgo, 0, false, 1);

        } else {

          while (ptr > 0 && now() >= vc[0].tstart) {

            rcoff.push_back(vc[0].pin);
            relay[vc[0].pin] = (!(rstat[vc[0].pin]));
            if (ptr > 0) delC();
          }

        }
        save = true;
      }


      int xskip = -1;
      for (int i = 0; i < Xfunc.size(); i++) {

        uint8_t pin = Xfunc[i].pin;
        uint8_t  id = Xfunc[i].xid;
        xoff[pin] = Xfunc[i].offtime;

        bool isOn = rstat[pin];

        if (relay[pin] == isOn || xskip == id) continue;             // Если уже стоит ;)

        if (startt[pin] == 0) {

          startt[pin] = 1; tmpr[pin] = 1; xoldid[pin] = id;
          //   sendEvent(">>>"+String(pin,DEC)+":"+String(xoldid[pin],DEC)+":"+String(id,DEC),"sms");

        }

        if (xoldid[pin] != id) {
          xoldid[pin] = id;
          startt[pin]++;
          tmpr[pin] = (tmpr[pin] || mycheck(i));
          if (!tmpr[pin]) {
            xskip = id;
            continue;
          }

        } else {
          xoldid[pin] = id;
          tmpr[pin] = (tmpr[pin] *  mycheck(i));
          if (!tmpr[pin]) {
            xskip = id;
            continue;
          }
        };
      };


      for (int i = 0; i < 8; i++) {
        bool isOn = rstat[i];
        if (startt[i] == 0 ) continue;

        if (tmpr[i] && relay[i] != isOn) {
          //  sendEvent("SHIT!>>"+String(i)+":"+String(Xfunc[i].offtime)+":"+String(tmpr[i])+":"+String(relay[i]),"sms");
          if (conf.pcfr_active) pcf1.write(conf.pcfr_swap[i], isOn);
          pix(i, isOn);

          relay[i] = isOn;
          rchange = true;
          addCron(i, now() + xoff[i] - 1, 0, false, 1);
          save = true;

        }
      }

      while (!rcoff.empty()) {
        uint8_t rec = rcoff.back();
        rcoff.pop_back();
        if (tmpr[rec]) continue;
        relay[rec] = !(rstat[rec]);
        if (conf.pcfr_active) pcf1.write(conf.pcfr_swap[rec], (!(rstat[conf.pcfr_swap[rec]])));
        pix(rec, (!(rstat[rec])));
        rchange = true;
      }


      if (save)  xsaveState();
      if (rchange) {
        sendR(false);
      }

    }



    void startIFTT() {

      File f;

      if (SPIFFS.exists("/admin/ini/iftt.ini")) f = SPIFFS.open("/admin/ini/iftt.ini", "r"); else return;
      if (!f) {
        Serial.println("Failed to open state.ini");
        return;
      }
      Xfunc.clear();
      int id = 0;
      while (f.available()) {
        String xs = f.readStringUntil('\n');
        if (xs.substring(0, 1) == "#" || xs.length() < 5) continue;
        uint8_t sep = xs.indexOf("=");
        if (!sep) continue;
        String tmp = xs.substring(0, sep);
        String val = xs.substring(sep + 1); val.trim();
        sep = tmp.indexOf(":");
        uint8_t pin = xs.substring(0, sep).toInt();
        int rtime = xs.substring(sep + 1).toInt();
        std::vector<String>vc;
        char *token = strtok(&val[0], "|");
        while (token) {
          vc.push_back(token);
          token = strtok(NULL, "|");
        };
        for (uint8_t i = 0; i < vc.size(); i++) {
          setTask(vc[i], pin, rtime, id);
        }
        id++;
        val = String(); vc.clear();
        std::vector<String>(vc).swap(vc);
      }
      f.close();

      doCron();
    }


    void showActive() {
      char buff[200]; snprintf_P(buff, countof(buff), line); charEvent(buff);
      snprintf_P(buff, countof(buff), txt4); charEvent(buff);
      snprintf_P(buff, countof(buff), line); charEvent(buff);
      for (uint8_t i = 0; i < ptr; i++) {
        int xtime = vc[i].tstart - now();
        if (xtime < 0) xtime = xtime * -1;
        snprintf_P(buff, countof(buff), txt5, i, vc[i].pin, printDateTimeT(vc[i].tstart).c_str(), secTime(xtime).c_str()); charEvent(buff);
      }
      snprintf_P(buff, countof(buff), line); charEvent(buff);
    }

    void show(uint8_t from, uint8_t to, int8_t rel) {

      int oldd = 0;
      char buff[100];

      snprintf_P(buff, countof(buff), line); charEvent(buff); //PRINT LINE

      snprintf_P(buff, countof(buff), " >>> %02u ", Xfunc.size()); charEvent(buff);

      if (to > Xfunc.size()) to = Xfunc.size();
      for (uint8_t zi = from; zi < to; zi++) {

        if (rel > -1) if (Xfunc[zi].pin != rel) continue;

        char buff[100];
        const char *xx[2] = {"OFF", " ON"};
        uint8_t rx =  mycheck(zi);
        //  Serial.println(zi);


        if (oldd != Xfunc[zi].xid) {
          //buff[0]=0;
          snprintf_P(buff, countof(buff), line); charEvent(buff);
          //buff[0]=0;
        }

        oldd = Xfunc[zi].xid;
        time_t t = now() + 3600 * conf.timeZone;
        snprintf_P(buff,
                   100,
                   //           PSTR("ID: %3u; TaskID : %02d ;  pin : %02d;   sensor : %02d; STATUS : %s"),
                   "Time: %02u:%02u:%02u  ID : %3d  ; TaskID : %2u;  pin : %2u;  sensor : %3u; STATUS : %s",
                   hour(t),
                   minute(t),
                   second(t),
                   zi,
                   Xfunc[zi].xid,
                   Xfunc[zi].pin,
                   Xfunc[zi].sensor,
                   xx[rx]
                  );

        //    Serial.println(String(buff));
        charEvent(buff);

      }


      return;

    }



} cron;

*/