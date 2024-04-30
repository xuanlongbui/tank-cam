#include "Arduino.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
// #include <SPIFFS.h>
#include <WiFi.h>
#include "esp_camera.h"
#include <ArduinoJson.h>
#include "esp_timer.h"
#include "img_converters.h"

#include "fb_gfx.h"
#include "soc/soc.h"          // disable brownout problems
#include "soc/rtc_cntl_reg.h" // disable brownout problems
#include "esp_http_server.h"
// #include "SPIFFS.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
// direct control
#define STOP 0
#define FORWARD 1
#define BACKWARD 2
#define RIGHT 3
#define LEFT 4
// AI Thinker
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define PART_BOUNDARY "123456789000000000000987654321"

static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

StaticJsonDocument<200> doc;
StaticJsonDocument<200> docOut;

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>

<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <style>
    * {
    touch-action: manipulation;
  }

* {
    -webkit-touch-callout: none;
    /* iOS Safari */
    -webkit-user-select: none;
    /* Safari */
    -khtml-user-select: none;
    /* Konqueror HTML */
    -moz-user-select: none;
    /* Old versions of Firefox */
    -ms-user-select: none;
    /* Internet Explorer/Edge */
    user-select: none;
    /* Non-prefixed version, currently
                                supported by Chrome, Edge, Opera and Firefox */
  }

  body {
    font-family: Arial;
    text-align: center;
    margin: 0px auto;
    padding-top: 30px;
  }

  table {
    margin-left: auto;
    margin-right: auto;
  }

  td {
    padding: 8 px;
  }
input{
  max-width: 100px;
}
  .button {
    background-color: #2f4468;
    border: none;
    color: white;
    padding: 10px 20px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 18px;
    margin: 6px 6px;
    text-align: center;
    cursor: pointer;
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    -webkit-tap-highlight-color: rgba(0, 0, 0, 0);
  }

  .button_speed {
    background-color: #2f4468;
    border: none;
    color: white;
    padding: 10px 20px;
    position: relative;
    right: 50%;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 18px;
  }


  .container {
    display: grid;
    margin: 0 auto;
    max-width: fit-content;
    grid-template-columns: 10% 80% 10%;

  }
  .speed-indicator {
      flex-grow: 1;
      height: 10px;
      background: #ccc;
      margin: 0 10px;
  }

  .noselect {
    max-width: 200px;
  }
  img {
    width: 352px;
    max-width: 100%;
    height: 240px;
  }
  ul {
    list-style-type: none;
    margin: 0;
    padding: 0;
    overflow: hidden;
    border: 1px solid #e7e7e7;
    background-color: #f3f3f3;
    width: 100%;
    height: 40px;
  }
  li {
    float: left;
    width: 50%;
  }

  li a {
    display: block;
    color: #666;
    text-align: center;
    padding: 14px 16px;
    text-decoration: none;
  }

  li a:hover:not(.active) {
    background-color: #ddd;
    font-size: 14px;
  }

  li a.active {
    color: white;
    background-color: #737b7b;
    font-size: 14px;
    
  }
  </style>
</head>

<body>
  <script>
    let obj = {
      "type":"control",
      "contents":{
        'direct': 'stop',
        'speed1': 50,
        'speed2': 50
      }
    }
    async function driectControl(param) {
      obj.contents.direct = param;
      console.log(obj);

      try {
        const response = await fetch("http://tankServer.local/control", {
          method: "POST",
          body: JSON.stringify(obj),
          headers: {
            "Content-Type": "application/json",
          },
        });
      } catch (error) {
        alert("Request failed - check the console");
        console.error(error);
      }
    }
    function speedControl1(param1) {
      let speed  = obj.contents.speed1 + param1;
      if (speed >= 0 && speed <= 255) {
        obj.contents.speed1 += param1;
      }
        document.getElementById("textSliderValue1").innerHTML = "MOTOR 1: " + obj.contents.speed1;
    }
    function speedControl2(param2) {
      let speed  = obj.contents.speed2 + param2;
      if (speed >= 0 && speed <= 255) {
        obj.contents.speed2 += param2;
        
      }
      document.getElementById("textSliderValue2").innerHTML = "MOTOR 2: " + obj.contents.speed2;
    }
    async function submitConfig() {
      let obj = {
        "type":"config",
        "contents":{
          "motor1":{
            "init_val": Number(document.getElementById("init_val_1").value),
            "stand_by_value": Number(document.getElementById("stand_by_val_1").value),
            "init_time":Number(document.getElementById("init_time_1").value),
            "pin":Number(document.getElementById("pin_1").value)
          },
          "motor2":{
            "init_val": Number(document.getElementById("init_val_2").value),
            "stand_by_value": Number(document.getElementById("stand_by_val_2").value),
            "init_time":Number(document.getElementById("init_time_2").value),
            "pin":Number(document.getElementById("pin_2").value)
          }
        }
      }
      console.log(obj);
      try {
        const response = await fetch("http://tankServer.local/config", {
          method: "POST",
          body: JSON.stringify(obj),
          headers: {
            "Content-Type": "application/json",
          },
        });
      } catch (error) {
        alert("Request failed - check the console");
        console.error(error);
      }
    }
  </script>
  <img src="" id="photo">
  <div class="container">
    <div>
      <button class="button_speed"  ontouchstart="speedControl1(-5)" >-</button>
    </div>
    <div class="noselect">
      <p><span id="textSliderValue1" > MOTOR 1: 50</span></p>
    </div>

    <div>
      <button class="button_speed" id="speed-up" ontouchstart="speedControl1(5)">+</button>
    </div>
  </div>

  <div class="container">
    <div>
      <button class="button_speed"  ontouchstart="speedControl2(-5)" >-</button>
    </div>
    <div class="noselect">
      <p><span id="textSliderValue2" > MOTOR 2: 50</span></p>
    </div>

    <div>
      <button class="button_speed" id="speed-up" ontouchstart="speedControl2(5)">+</button>
    </div>

  </div>

  <table>
    <tr>
      <td colspan="3" align="center"><button class="button" onmousedown="driectControl('forward');"
          ontouchstart="driectControl('forward');" onmouseup="driectControl('stop');"
          ontouchend="driectControl('stop');">Forward</button></td>
    </tr>
    <tr>
      <td ><button class="button" ontouchstart="driectControl('left');"
          ontouchend="driectControl('stop');">Left</button></td>
      <td ><button class="button" ontouchstart="driectControl('stop');"
          ontouchstart="driectControl('stop');">Stop</button></td>
      <td ><button class="button" ontouchstart="driectControl('right');"
          ontouchend="driectControl('stop');">Right</button></td>
    </tr>
    <tr>
    </tr>
  </table>
  <table>
    <tr>
      <td class="title" > MOTOR 1 </td>
      <td class="title"> MOTOR 2 </td>
    </tr>
    <table></table>
    <table>
      <tr>
        <td class="cl_title"> Initial value: </td>
        <td> <input type="number" id="init_val_1" placeholder="150" min="0" max="180" value="150"> </td>
        <td class="cl_title"> Initial value: </td>
        <td> <input type="number" id="init_val_2" placeholder="150" min="0" max="180" value="150"> </td>
      </tr>
      <tr>
        <td> Stand by value:</td>
        <td> <input type="number" id="stand_by_val_1" placeholder="20" min="0" max="180" value="20"> </td>
        <td> Stand by value:</td>
        <td> <input type="number" id="stand_by_val_2" placeholder="20" min="0" max="180" value="20"> </td>
      </tr>
      <tr>
        <td> Launch delay (ms):</td>
        <td> <input type="number" id="init_time_1" placeholder="200" min="0" max="500" value="200"> </td>
        <td> Launch delay (ms): </td>
        <td> <input type="number" id="init_time_2" placeholder="200" min="0" max="500" value="200"> </td>
      </tr>
      <tr>
        <td>Pin motor: </td>
        <td> <input type="number" id="pin_1" placeholder="3" min="0" max="80" value ="3"> </td>
        <td>Pin motor: </td>
        <td> <input type="number" id="pin_2" placeholder="5" min="0" max="80" value="5"> </td>
      </tr>
    </table>
    <div id="bt">
      <button class="button" onclick="submitConfig()">Submit</button>
    </div>
  <script>
    window.onload = document.getElementById("photo").src = "http://tankServer.local:81/stream";
  </script>
</body>

</html>
)rawliteral";

static esp_err_t control_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
  // return httpd_resp_send(req, (const char *)ctr.read(), strlen((const char *)ctr.read()));
}
static esp_err_t stream_handler(httpd_req_t *req)
{
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK)
  {
    return res;
  }

  while (true)
  {
    fb = esp_camera_fb_get();
    if (!fb)
    {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    }
    else
    {
      if (fb->width > 400)
      {
        if (fb->format != PIXFORMAT_JPEG)
        {
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted)
          {
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        }
        else
        {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if (res == ESP_OK)
    {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb)
    {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    }
    else if (_jpg_buf)
    {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK)
    {
      break;
    }
    // Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}
void uart_cmd(String msg_input){
  int direction = 0;
    // Serial.println(msg_input);
    DeserializationError error = deserializeJson(doc, msg_input);
    // Test if parsing succeeds.
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
    const char* direct = doc["direct"];
    // Serial.println(direct);
    
    if (strcmp(doc["direct"],"stop") == 0)
    {
      direction = STOP;
    }
    else if (strcmp(doc["direct"],"forward") == 0)
    {
      direction = FORWARD;
    }
    else if (strcmp(doc["direct"],"backward") == 0)
    {
      direction = BACKWARD;
    }
    else if (strcmp(doc["direct"],"right") == 0)
    {
      direction = RIGHT;
    }
    else if (strcmp(doc["direct"],"left") == 0)
    {
      direction = LEFT;
    }
    else
    {
      //
    }
    docOut["direct"] = direction;
    String c_speed1 = doc["speed1"];
    String c_speed2 = doc["speed2"];

    docOut["speed1"] = c_speed1.toInt() ;
    docOut["speed2"] = c_speed2.toInt();

    serializeJson(docOut, Serial);
    Serial.println();
}
static esp_err_t cmd_handler(httpd_req_t *req)
{
  /* Destination buffer for content of HTTP POST request.
   * httpd_req_recv() accepts char* only, but content could
   * as well be any binary data (needs type casting).
   * In case of string data, null termination will be absent, and
   * content length would give length of string */
  char content[100];
  int ind_ct = 0;
  char character;
  String output_str = "";
  /* Truncate if content length larger than the buffer */
  size_t recv_size = MIN(req->content_len, sizeof(content));

  int ret = httpd_req_recv(req, content, recv_size);
  if (ret <= 0)
  { /* 0 return value indicates connection closed */
    /* Check if timeout occurred */
    if (ret == HTTPD_SOCK_ERR_TIMEOUT)
    {
      /* In case of timeout one can choose to retry calling
       * httpd_req_recv(), but to keep it simple, here we
       * respond with an HTTP 408 (Request Timeout) error */
      httpd_resp_send_408(req);
    }
    /* In case of error, returning ESP_FAIL will
     * ensure that the underlying socket is closed */
    return ESP_FAIL;
  }
  else
  {
    int cnt =0;
    for (int i = 0; i < 100; i++)
    {
      character = content[i];
      if (character != '}')
      {
        output_str.concat(character);
      }
      else
      {
        output_str.concat(character);
        cnt ++;
        if (cnt == 2 )
        {
          break;
        }
        
      }
    }
  Serial.println(output_str);
  // uart_cmd(output_str);
  }

  /* Send a simple response */
  const char resp[] = "URI POST Response";
  httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}
static esp_err_t cfg_handler(httpd_req_t *req)
{
  /* Destination buffer for content of HTTP POST request.
   * httpd_req_recv() accepts char* only, but content could
   * as well be any binary data (needs type casting).
   * In case of string data, null termination will be absent, and
   * content length would give length of string */
  char content[200];
  int ind_ct = 0;
  char character;
  String output_str = "";
  /* Truncate if content length larger than the buffer */
  size_t recv_size = MIN(req->content_len, sizeof(content));

  int ret = httpd_req_recv(req, content, recv_size);
  if (ret <= 0)
  { /* 0 return value indicates connection closed */
    /* Check if timeout occurred */
    if (ret == HTTPD_SOCK_ERR_TIMEOUT)
    {
      /* In case of timeout one can choose to retry calling
       * httpd_req_recv(), but to keep it simple, here we
       * respond with an HTTP 408 (Request Timeout) error */
      httpd_resp_send_408(req);
    }
    /* In case of error, returning ESP_FAIL will
     * ensure that the underlying socket is closed */
    return ESP_FAIL;
  }
  else
  {
    int cnt =0;
    for (int i = 0; i < 200; i++)
    {
      character = content[i];
      if (character != '}')
      {
        output_str.concat(character);
      }
      else
      {
        output_str.concat(character);
        cnt ++;
        if (cnt == 4 )
        {
          break;
        }
        
      }
    }
  Serial.println(output_str);
  // uart_cmd(output_str);
  }

  /* Send a simple response */
  const char resp[] = "URI POST Response";
  httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

void startCameraServer()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t control_uri = {
      .uri = "/",
      .method = HTTP_GET,
      .handler = control_handler,
      .user_ctx = NULL};
  httpd_uri_t cmd_uri = {
      .uri = "/control",
      .method = HTTP_POST,
      .handler = cmd_handler,
      .user_ctx = NULL};
  httpd_uri_t config_uri = {
      .uri = "/config",
      .method = HTTP_POST,
      .handler = cfg_handler,
      .user_ctx = NULL};
  httpd_uri_t stream_uri = {
      .uri = "/stream",
      .method = HTTP_GET,
      .handler = stream_handler,
      .user_ctx = NULL};
  if (httpd_start(&camera_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler(camera_httpd, &control_uri);
    httpd_register_uri_handler(camera_httpd, &config_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
  Serial.begin(9600);
  Serial.setDebugOutput(true);
   camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound())
  {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  }
  else
  {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Serial.println("Setting AP (Access Point)");
  // NULL sets an open Access Point
  WiFi.softAP("ESP-WIFI", NULL);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  MDNS.begin("tankServer");
  // Start streaming web server
  startCameraServer();
}

void loop()
{
  // Do nothing. Everything is done in another task by the web server
  // delay(1000);
  // Serial.println("{\"direct\":1, \"speed1\":100, \"speed2\":100}");
  // delay(100);
  // Serial.println("{\"direct\":0, \"speed1\":200, \"speed2\":200}");
}
