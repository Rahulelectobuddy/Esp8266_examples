#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char* ssid = "rahul";
const char* password = "micromax";

int refresed=0,change=0;
String default1OFF="checked",default1ON="",default2OFF="checked",default2ON="";

ESP8266WebServer server(80);

//Check if header is present and correct
bool is_authentified(){
  Serial.println("Enter is_authentified");
  if (server.hasHeader("Cookie")){
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentification Successful");
      return true;
    }
  }
  Serial.println("Authentification Failed");
  return false;
}

//login page, also called for disconnect
void handleLogin(){
  String msg;
  if(refresed){
    if(change){
      msg = "<br><font color='red'>CHANGES SAVED</font><br>"; change=0;}
    else
      msg = "<br><font color='red'>TIMEOUT<br>Please Login again</font><br>";
    refresed=0;
  }
  refresed=0;
  if (server.hasArg("DISCONNECT")){
    Serial.println("Disconnection");
    server.sendHeader("Location","/login");
    server.sendHeader("Cache-Control","no-cache");
    server.sendHeader("Set-Cookie","ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")){
    if (server.arg("USERNAME") == "admin" &&  server.arg("PASSWORD") == "admin" ){
      server.sendHeader("Location","/change");
      server.sendHeader("Cache-Control","no-cache");
      server.sendHeader("Set-Cookie","ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
   msg = "<br><font color='red'>Wrong username/password! try again.</font><br>";
   Serial.println("Log in Failed");
  }
  String content = "<html><body style='background-color:aliceblue;'><br><br><br><br><center><fieldset style='width:380px'><legend><h2>LOGIN PAGE:&nbsp&nbsp</h2></legend><br><font color='blue'>All field with '*' are mandatory</font><br><br><form action='/login' method='POST'>Name&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp&nbsp:&nbsp&nbsp<input type='text' name='NAME' placeholder=' User name' size='10'><br><br>User ID *&nbsp&nbsp&nbsp:&nbsp&nbsp<input type='text' name='USERNAME' placeholder=' User number' size='10'><br><br>Password *:&nbsp&nbsp<input type='password' name='PASSWORD' placeholder=' Password' size='10'><br>";
  content+= msg+"<br><input type='submit' name='SUBMIT' value='Submit'></form></fieldset></center></body></html>";
  server.send(750, "text/html", content);
}

void handlechange(){
  if (server.hasArg("LED1") && server.hasArg("LED2")){
    if (server.arg("LED1") == "ON")
      {Serial.println("led1 is on");change++;default1ON="checked";default1OFF="";}
    else
      {Serial.println("led1 is off");change++;default1OFF="checked";default1ON="";}
    if(server.arg("LED2") == "ON")
      {Serial.println("led2 is on");change++;default2ON="checked";default2OFF="";}
    else
      {Serial.println("led2 is off");change++;default2OFF="checked";default2ON="";}
   }
   if(refresed)
   {
    server.sendHeader("Location","/login");
    server.sendHeader("Cache-Control","no-cache");
    server.sendHeader("Set-Cookie","ESPSESSIONID=0");
    server.send(301);
    return;
    }
  String content = "<html><head><meta http-equiv='refresh' content='10'></head><body style='background-color:aliceblue;'><br><br><br><br><center><h2>WEBPAGE:&nbsp&nbsp</h2><form action='/change' method='GET'><fieldset id='LED1' style='width:300px'><legend><h3>LED1:&nbsp&nbsp</h3></legend>ON<input type='radio' value='ON' name='LED1'";
  content += default1ON+">OFF<input type='radio' value='OFF' name='LED1'";
  content += default1OFF+"></fieldset><fieldset id='LED2' style='width:300px'><legend><h3>LED2:&nbsp&nbsp</h3></legend>ON<input type='radio' value='ON' name='LED2'";
  content += default2ON+">OFF <input type='radio' value='OFF' name='LED2'";
  content += default2OFF+"></fieldset><br><br><input type='submit' name='SUBMIT' value='CHANGES DONE'></form><br>You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html></center></body></html>";
  server.send(750, "text/html", content);
  refresed++;
}

//root page can be accessed only if authentification is ok
void handleRoot(){
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authentified()){
    server.sendHeader("Location","/login");
    server.sendHeader("Cache-Control","no-cache");
    server.send(301);
    return;
  }
  String content = "<html><body><H2>hello, you successfully connected to esp8266!</H2><br>";
  if (server.hasHeader("User-Agent")){
    content += "the user agent used is : " + server.header("User-Agent") + "<br><br>";
  }
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(200, "text/html", content);
}

//no need authentification
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  server.on("/", handleRoot);
  server.on("/login", handleLogin);
  server.on("/change", handlechange);
 
  server.onNotFound(handleNotFound);
  //here the list of headers to be recorded
  const char * headerkeys[] = {"User-Agent","Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  //ask server to track these headers
  server.collectHeaders(headerkeys, headerkeyssize );
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
