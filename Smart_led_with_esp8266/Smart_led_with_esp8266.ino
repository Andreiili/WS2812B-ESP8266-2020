// https://www.youtube.com/watch?v=SkBlp5-VkGM

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

RgbColor white(255, 255, 255);
RgbColor black(0, 0, 0);


ESP8266WebServer server(80);


String nume ;// numele care va aparea la antet
bool informatii_introduce = 0; // daca s-au introdus informatiile atunci se va a fisa meniul de control





uint8_t pixel, dinapoi; // unde se vor stoca pozitia care se lumineaza pentru fiecare led , imi face scara dubla , din amandoua partile imi lumineaza
byte  red, green, blue, timp;//date pentru stocarea din pagina web , reg,green,blue ia culorile pentru astea si timp ia in cat timp se va face tot ciclul
byte mod_aprindere;//aici se va stoca ce culoare se va arpinde cand se va activa , 1 == white , 2== Color , 3 ==  Fade In Fade Out iar 0 == off

bool negru = 1;
bool  alb, fade_in_fade_out, culoare, up_down, ora;//imi pune in pagina efectiv
bool  alb_ora, negru_ora, fade_in_fade_out_ora, culoare_ora; //pentru ora se stocheaza
bool adunare_punctaj = true ; // asta este pentru atunci cand imi adauga sau scade pixelul , este facut fara for , deoarece astreapta sa mi-se termine for-ul ca sa se modifice

bool culoare_buton; //atunci cand am trimis codul sa imi afiseze butonul de off

byte ora_off, ora_on = 0;//aici imi stocheaza informatia la ce ora mi-se incepe ciclu si se inchide
byte minut_off, minut_on = 0;
long timp_pixel = 500;//timpul pe care il face intre led-uri
const long utcOffsetInSeconds = 7200;// timp setat ceasul

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);



boolean fadeToColor = true;
const uint16_t PixelCount = 7;
uint8_t PixelPin = 3;
const uint8_t AnimationChannels = 1;


NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
NeoPixelAnimator animations(AnimationChannels);


struct MyAnimationState
{
  RgbColor StartingColor;
  RgbColor EndingColor;
};

MyAnimationState animationState[AnimationChannels];

void SetRandomSeed()
{
  uint32_t seed;
  seed = analogRead(0);
  delay(1);
  for (int shifts = 3; shifts < 31; shifts += 3)
  {
    seed ^= analogRead(0) << shifts;
    delay(1);
  }

  randomSeed(seed);
}

void BlendAnimUpdate(const AnimationParam& param)
{
  RgbColor updatedColor = RgbColor::LinearBlend(
                            animationState[param.index].StartingColor,
                            animationState[param.index].EndingColor,
                            param.progress);


  bool timp_delay = 1;
  if ( (animationState[param.index].StartingColor == 0) && (animationState[param.index].EndingColor == 0) && (param.progress == 0) )
    timp_delay = 0;

  if (up_down == 1) {
    if (adunare_punctaj) {
      strip.SetPixelColor(pixel, updatedColor);
      strip.SetPixelColor(dinapoi, updatedColor);
      if (timp_delay)
        delay(timp_pixel);
      strip.Show();
    }
  }
  else  for (uint16_t pixel = 0; pixel <= PixelCount ; pixel++)
    {
      strip.SetPixelColor(pixel, updatedColor);
    }
}

void FadeInFadeOutRinseRepeat(float luminance)
{
  if (fadeToColor)
  {
    delay(1000);
    RgbColor target = HslColor(random(360) / 360.0f, 1.0f, luminance);
    uint16_t time = random(800, 2000);
    animationState[0].StartingColor = strip.GetPixelColor(0);
    animationState[0].EndingColor = target;
    animations.StartAnimation(0, time, BlendAnimUpdate);
  }
  else
  {
    uint16_t time = random(600, 700);
    animationState[0].StartingColor = strip.GetPixelColor(0);
    animationState[0].EndingColor = RgbColor(0);
    animations.StartAnimation(0, time, BlendAnimUpdate);
  }
  fadeToColor = !fadeToColor;
}






String Log_in(bool intrebare) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Modify</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Hello</h1>\n";
  ptr += "<p>Please introduce the name should you refer to me </p>\n";
  ptr += "<FORM METHOD='POST'action='/data'>\n";// trebuie sa am o pagina ca sa se execute cand intra pe pagina /color_send
  ptr += "<input type='text' name='nume'  style=' -moz-appearance: textfield' value=" + String(nume) + " >\n";
  ptr += "<input type='submit' value='Send the info'>\n";
  ptr += "</form>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}



String Pagina_Web(bool alb , bool negru , bool fade_in_fade_out , bool culoare  , bool up_down, bool ora) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>" + nume + "</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #1abc9c;}\n";
  ptr += ".button-on:active {background-color: #16a085;}\n";
  ptr += ".button-off {background-color: #77878A;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>" + nume + "</h1>\n";
  if (negru == 0)
    ptr += "<p>Lamp - State On</p><a class=\"button button-off\" href=\"/disable\">OFF</a>\n";
  else
    ptr += "<p>Lamp - State Off</p>\n";
  if (alb == 1)
    ptr += "<p>White light - State On</p><a class=\"button button-off\" href=\"/disable\">OFF</a>\n";
  else
    ptr += "<p>White light - State Off</p><a class=\"button button-on\" href=\"/White_light_on\">ON</a>\n";
  if (culoare_buton == 1 )
    ptr += "<p>Color - State On</p><a class=\"button button-off\" href=\"/Color_on\">MODIFY</a>\n";
  else if (culoare == 1)
  { ptr += "<p>Color - State On</p>\n";
    ptr += "<FORM METHOD='POST'action='/Color_off'>\n";// trebuie sa am o pagina ca sa se execute cand intra pe pagina /color_send
    ptr += "<p>Enter the rgb color(0-255)</p>\n";
    ptr += "<p>Red Code</p><input type='number' name='red'  style='width: 2em ; -moz-appearance: textfield' value=" + String(red) + " min='0' max='255'>\n";
    ptr += "<p>Green Code</p><input type='number' name='green'  style='width: 2em ; -moz-appearance: textfield' value=" + String(green) + " min='0' max='255'>\n";
    ptr += "<p>Blue Code</p><input type='number' name='blue' style='width: 2em ; -moz-appearance: textfield' value=" + String(blue) + " min='0' max='255'>\n";
    ptr += "<p>Last value:</p>\n";
    ptr += "<input type='submit' value='Send RGB code'>\n";
    ptr += "</form>\n";
  }
  else
    ptr += "<p>Color - State Off</p><a class=\"button button-on\" href=\"/Color_on\">ON</a>\n";
  if (fade_in_fade_out == 1)
    ptr += "<p>Fade In Fade Out - State On</p><a class=\"button button-off\" href=\"/Fade_In_Fade_Out_off\">OFF</a>\n";
  else
    ptr += "<p>Fade In Fade Out - State Off</p><a class=\"button button-on\" href=\"/Fade_In_Fade_Out_on\">ON</a>\n";
  if (up_down == 1)
  {
    ptr += "<FORM METHOD='POST'action='/up_down_on'>\n";// trebuie sa am o pagina ca sa se execute cand intra pe pagina /color_send
    ptr += "<p>Enter the time to make a full circle(in seconds)</p>\n";
    ptr += "<p>Time</p><input type='number' name='timp' style='width: 2em ; -moz-appearance: textfield' value=" + String(timp) + " min='0' max='255'></p>\n";
    ptr += "<input type='submit' value='Send The Time'>\n";
    ptr += "</form>\n";
    ptr += "<p>Set The State Off of the Led Ladder</p><a class=\"button button-off\" href=\"/up_down_off\">OFF</a>\n";
  }
  else
    ptr += "<p>Led Ladder - State Off</p><a class=\"button button-on\" href=\"/up_down_on\">ON</a>\n";
  if (( (ora_on >= 1) || (minut_on >= 1) || (minut_off >= 1) || (ora_off >= 1)) && (ora == 1))
    ptr += "<p> Time Led- State On</p><a class=\"button button-off\" href=\"/ora_off\">OFF</a>\n";
  else if (ora == 1)
  {
    ptr += "<FORM METHOD='POST'action='/ora_on'>\n";
    ptr += "<p>The time when the led's will turn on or off (00:00-24:00)</p>\n";
    ptr += "<p>Lights On</p><input type='number' name='ora_on' style='width: 2em ; -moz-appearance: textfield' value=" + String(ora_on) + " min='0' max='24'>:<input type='number' name='minut_on' style='width: 2em ; -moz-appearance: textfield' value=" + String(minut_on) + " min='0' max='60'>\n";
    ptr += "<p>Lights Off</p><input type='number' name='ora_off' style='width: 2em ; -moz-appearance: textfield' value=" + String(ora_off) + " min='0' max='24'>:<input type='number' name='minut_off' style='width: 2em ; -moz-appearance: textfield' value=" + String(minut_off) + " min='0' max='60'>\n";
    ptr += "<p>Select the light mode to start</p>\n";
    ptr += "<p>White Light "   "<input type='radio'  name='alb' value='1' >\n";
    ptr += "<p>Color "   "<input type='radio'  name='culoare' value='1' >\n";
    ptr += "<p>Fade In Fade Out "   "<input type='radio'  name='fade_in_fade_out' value='1'  ></p>\n";
    ptr += "<input type='submit' value='Send The Time Range' >\n";
    ptr += "</form>\n";
  }
  else
    ptr += "<p> Time Led- State Off</p><a class=\"button button-on\" href=\"/ora_on\">ON</a>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

void setup(void) {
  Serial.begin(115200);
  strip.Begin();
  strip.Show();
  SetRandomSeed();
  delay(100);

  /* WiFi.begin(ssid, password);
    while ( WiFi.status() != WL_CONNECTED ) {
     delay ( 500 );
     Serial.print ( "." );
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();

  */

  WiFiManager wifiManager;

  wifiManager.resetSettings();

  wifiManager.autoConnect("Light Press Notification ");

  timeClient.begin();








  server.on("/data", []() {

    nume = server.arg("nume");
    informatii_introduce = 1;
    server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));
  });


  server.on("/", []() {

    if (informatii_introduce)
      server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));
    else
      server.send(200, "text/html", Log_in(0));

  });






  server.on("/Color_off", []() {
    fade_in_fade_out = 0;
    alb = 0;
    negru = 0;
    culoare = 1;
    culoare_buton = 1;
    red = server.arg("red").toInt();
    green = server.arg("green").toInt();
    blue = server.arg("blue").toInt();
    RgbColor color = RgbColor(red, green, blue);
    server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));

  });


  server.on("/Color_on", []() {
    fade_in_fade_out = 0;
    alb = 0;
    negru = 0;
    culoare = 1;
    culoare_buton = 0;
    server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));
  });

  server.on("/Fade_In_Fade_Out_off", []() {
    fade_in_fade_out = 0;
    alb = 0;
    negru = 0;
    culoare = 0;
    culoare_buton = 0;
    server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));
  });

  server.on("/disable", []() {
    fade_in_fade_out = 0;
    alb = 0;
    negru = 1;
    culoare = 0;
    culoare_buton = 0;
    server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));
  });

  server.on("/Fade_In_Fade_Out_on", []() {
    alb = 0;
    negru = 0;
    culoare = 0;
    fade_in_fade_out = 1;
    culoare_buton = 0;
    server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));
  });

  server.on("/White_light_on", []() {
    fade_in_fade_out = 0;
    negru = 0;
    culoare = 0;
    alb = 1;
    culoare_buton = 0;
    server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));
  });



  server.on("/up_down_on", []() {
    up_down = 1;
    pixel = 0;
    timp = server.arg("timp").toInt();
    timp_pixel = (timp * 1000) / PixelCount;
    server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));
  });

  server.on("/up_down_off", []() {
    up_down = 0;
    pixel = 0;
    server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));
  });






  server.on("/ora_on", []() {
    ora = 1;
    ora_on = server.arg("ora_on").toInt();
    minut_on = server.arg("minut_on").toInt();

    ora_off = server.arg("ora_off").toInt();
    minut_off = server.arg("minut_off").toInt();

    fade_in_fade_out_ora = server.arg("fade_in_fade_out").toInt();
    culoare_ora = server.arg("culoare").toInt();
    alb_ora = server.arg("alb").toInt();
    server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));
  });




  server.on("/ora_off", []() {
    ora = 0;
    server.send(200, "text/html", Pagina_Web(alb , negru , fade_in_fade_out , culoare, up_down, ora));
  });


  server.begin();



}

void loop(void) {
  server.handleClient();



  dinapoi = PixelCount - pixel;
  timeClient.update();



  if (timeClient.getHours() == ora_on)
    if (timeClient.getMinutes() == minut_on) {
      alb = alb_ora;
      culoare = culoare_ora;
      fade_in_fade_out = fade_in_fade_out_ora;
      negru = 0;
    }

  if (timeClient.getHours() == ora_off)
    if (timeClient.getMinutes() == minut_off) {
      alb = 0;
      culoare = 0;
      fade_in_fade_out = 0;
      negru = 1;
    }



  if (alb == 1)
  {
    if (up_down == 1) {


      if (adunare_punctaj) {
        strip.SetPixelColor(pixel, white);
        strip.SetPixelColor(dinapoi, white);
        delay(timp_pixel);
      }


      if (!adunare_punctaj) {
        strip.SetPixelColor(pixel, black);
        strip.SetPixelColor(dinapoi, black);
        delay(timp_pixel);
      }


    } else {
      for (uint8_t pixel = 0; pixel <= PixelCount; pixel++)
        strip.SetPixelColor(pixel, white);
    }
  }

  if (culoare == 1)
  {
    if (up_down == 1) {

      if (adunare_punctaj) {
        RgbColor color = RgbColor(red, green, blue);
        strip.SetPixelColor(pixel, color);
        strip.SetPixelColor(dinapoi, color);
        delay(timp_pixel);
      }


      if (!adunare_punctaj) {
        strip.SetPixelColor(pixel, black);
        strip.SetPixelColor(dinapoi, black);
        delay(timp_pixel);
      }



    } else {
      for (uint8_t pixel = 0; pixel <= PixelCount; pixel++) {
        RgbColor color = RgbColor(red, green, blue);
        strip.SetPixelColor(pixel, color);
      }
    }



  }


  if (fade_in_fade_out == 1)
  {
    if (up_down == 1) {

      if (animations.IsAnimating())
      {
        animations.UpdateAnimations();
      }
      else
      {
        FadeInFadeOutRinseRepeat(0.2f);
      }



      if (!adunare_punctaj) {
        strip.SetPixelColor(pixel, black);
        strip.SetPixelColor(dinapoi, black);
        delay(timp_pixel);
      }



    } else {
      if (animations.IsAnimating())
      {
        animations.UpdateAnimations();
        strip.Show();
      }
      else
      {
        FadeInFadeOutRinseRepeat(0.2f);
      }

    }
  }



  if (negru == 1 )
  {

    for (uint8_t pixel = 0; pixel <= PixelCount; pixel++)
      strip.SetPixelColor(pixel, black);

  }




  strip.Show();


  if (up_down == 1) {

    if (adunare_punctaj) {
      pixel++;
      if (pixel > (PixelCount / 2))
        adunare_punctaj = false;
    } else
    {
      pixel--;
      if (pixel == 0) {
        adunare_punctaj = true;
      }
    }
  }


}
