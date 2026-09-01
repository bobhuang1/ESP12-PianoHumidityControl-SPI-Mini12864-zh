#include "GarfieldCommon.h"





// Functions
String convertPoemNumberToFileName(int poemNumber, int TOTAL_POEMS) {
	String returnText = "";
	if (TOTAL_POEMS > 99)
	{
		if (poemNumber < 10)
		{
			returnText = "00" + String(poemNumber);
		}
		else if (poemNumber < 100)
		{
			returnText = "0" + String(poemNumber);
		}
		else
		{
			returnText = String(poemNumber);
		}
	}
	else
	{
		if (poemNumber < 10)
		{
			returnText = "0" + String(poemNumber);
		}
		else
		{
			returnText = String(poemNumber);
		}
	}
	return "/" + returnText + ".h";
}

String generateMathQuestion(String &Answer) {
	const String strPlusSign = "+";
	const String strMinusSign = "-";
	const String strMultiplySign = "X";
	const String strDivideySign = String((char)247);
	const String strEqualSign = "=";
	String MathQuestion = "";
	int intFirstOperationType = random(1, 4); // 1 - plus, 2 - minus, 3 - multiply
	int intSecondOperationType = 1;
	int intFirstNumber = 1;
	int intSecondNumber = 1;
	int intThirdNumber = 1;

	if (intFirstOperationType == 3)
	{
		intSecondOperationType = random(1, 3);
		intFirstNumber = random(1, 10);
		intSecondNumber = random(1, 10);
		if (intSecondOperationType == 2)
		{
			intThirdNumber = random(1, intFirstNumber * intSecondNumber);
			MathQuestion = String(intFirstNumber) + strMultiplySign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + "?";
			Answer = String(intFirstNumber) + strMultiplySign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber * intSecondNumber - intThirdNumber);
		}
		else
		{
			intThirdNumber = random(1, 100);
			MathQuestion = String(intFirstNumber) + strMultiplySign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + "?";
			Answer = String(intFirstNumber) + strMultiplySign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber * intSecondNumber + intThirdNumber);
		}
	}
	else if (intFirstOperationType == 2)
	{
		intSecondOperationType = random(1, 4);
		intFirstNumber = random(50, 100);
		if (intSecondOperationType == 1)
		{
			intSecondNumber = random(0, intFirstNumber);
			intThirdNumber = random(1, 100);
			MathQuestion = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + "?";
			Answer = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber - intSecondNumber + intThirdNumber);
		}
		else if (intSecondOperationType == 2)
		{
			intSecondNumber = random(30, intFirstNumber);
			intThirdNumber = random(1, (intFirstNumber - intSecondNumber));
			MathQuestion = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + "?";
			Answer = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber - intSecondNumber - intThirdNumber);
		}
		else // multiply
		{
			intSecondNumber = random(1, 10);
			intThirdNumber = random(1, 10);
			intFirstNumber = random(intSecondNumber * intThirdNumber, 100);
			MathQuestion = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strMultiplySign + String(intThirdNumber) + strEqualSign + "?";
			Answer = String(intFirstNumber) + strMinusSign + String(intSecondNumber) + strMultiplySign + String(intThirdNumber) + strEqualSign + String(intFirstNumber - (intSecondNumber * intThirdNumber));
		}
	}
	else // first operation is plus
	{
		intSecondOperationType = random(1, 4);
		intFirstNumber = random(1, 100);
		if (intSecondOperationType == 1)
		{
			intSecondNumber = random(1, 100);
			intThirdNumber = random(1, 100);
			MathQuestion = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + "?";
			Answer = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strPlusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber + intSecondNumber + intThirdNumber);
		}
		else if (intSecondOperationType == 2)
		{
			intSecondNumber = random(1, 100);
			intThirdNumber = random(1, (intFirstNumber - intSecondNumber));
			MathQuestion = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + "?";
			Answer = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strMinusSign + String(intThirdNumber) + strEqualSign + String(intFirstNumber + intSecondNumber - intThirdNumber);
		}
		else
		{
			intSecondNumber = random(1, 10);
			intThirdNumber = random(1, 10);
			MathQuestion = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strMultiplySign + String(intThirdNumber) + strEqualSign + "?";
			Answer = String(intFirstNumber) + strPlusSign + String(intSecondNumber) + strMultiplySign + String(intThirdNumber) + strEqualSign + String(intFirstNumber + (intSecondNumber * intThirdNumber));
		}
	}
	return MathQuestion;
}

void listSPIFFSFiles(void) {

	if (!SPIFFS.begin()) {
#ifdef DEBUG
		Serial.println("SPIFFS initialization failed!");
#endif
		while (1) yield(); // Stay here twiddling thumbs waiting
	}

#ifdef DEBUG
	Serial.println("\r\nInitialization done.");
#endif

#ifdef DEBUG
	Serial.println();
	Serial.println("SPIFFS files found:");
#endif
	fs::Dir dir = SPIFFS.openDir("/"); // Root directory
	String  line = "=====================================";

#ifdef DEBUG
	Serial.println(line);
	Serial.println("  File name               Size");
	Serial.println(line);
#endif

	while (dir.next()) {
		String fileName = dir.fileName();
#ifdef DEBUG
		Serial.print(fileName);
#endif
		int spaces = 25 - fileName.length(); // Tabulate nicely
		if (spaces < 0) spaces = 1;
#ifdef DEBUG
		while (spaces--) Serial.print(" ");
#endif
		fs::File f = dir.openFile("r");
#ifdef DEBUG
		Serial.print(f.size()); Serial.println(" bytes");
#endif
		yield();
	}
#ifdef DEBUG
	Serial.println(line);
	Serial.println();
#endif
	delay(1000);
}

char* string2char(String command) {
	if (command.length() != 0) {
		char *p = const_cast<char*>(command.c_str());
		return p;
	}
}

void shortBeep(int ALARMPIN, bool USE_HIGH_ALARM) {
	if (USE_HIGH_ALARM)
	{
		digitalWrite(ALARMPIN, HIGH);
		delay(150);
		digitalWrite(ALARMPIN, LOW);
	}
	else
	{
		digitalWrite(ALARMPIN, LOW);
		delay(150);
		digitalWrite(ALARMPIN, HIGH);
	}
}

void longBeep(int ALARMPIN, bool USE_HIGH_ALARM) {
	if (USE_HIGH_ALARM)
	{
		digitalWrite(ALARMPIN, HIGH);
		delay(2000);
		digitalWrite(ALARMPIN, LOW);
	}
	else
	{
		digitalWrite(ALARMPIN, LOW);
		delay(2000);
		digitalWrite(ALARMPIN, HIGH);
	}
}

void noBeep(int ALARMPIN, bool USE_HIGH_ALARM) {
	if (USE_HIGH_ALARM)
	{
		digitalWrite(ALARMPIN, LOW);
	}
	else
	{
		digitalWrite(ALARMPIN, HIGH);
	}
}

// Fill in your own SSID/password pairs here (or better, use USE_WIFI_MANAGER instead of
// hardcoding any of this - see the branch below). Never commit real WiFi credentials.
const char* const WIFI_TUNNEL_SSID[] = { "YOUR_TUNNEL_SSID" };
const char* const WIFI_TUNNEL_PWD[] = { "YOUR_TUNNEL_PASSWORD" };

const char* const WIFI_HOME_SSID[] = { "YOUR_SSID_1", "YOUR_SSID_2", "YOUR_SSID_3" };
const char* const WIFI_HOME_PWD[] = { "YOUR_PASSWORD_1", "YOUR_PASSWORD_2", "YOUR_PASSWORD_3" };

void connectWIFI(bool USE_WIFI_MANAGER, bool isTunnel) {
#define MAXIMUM_SSID 5
	const char* const* WIFI_SSID;
	const char* const* WIFI_PWD;
	int numWIFIs;
	if (isTunnel)
	{
		WIFI_SSID = WIFI_TUNNEL_SSID;
		WIFI_PWD = WIFI_TUNNEL_PWD;
		numWIFIs = 1;
	}
	else
	{
		WIFI_SSID = WIFI_HOME_SSID;
		WIFI_PWD = WIFI_HOME_PWD;
		numWIFIs = 3;
	}
#define WIFI_TRY 30
	if (USE_WIFI_MANAGER)
	{
		WiFi.persistent(true);
		WiFiManager wifiManager;
		wifiManager.setConfigPortalTimeout(600);
		wifiManager.autoConnect("ESP8266-Setup"); // rename to whatever AP name you want the setup portal to use
#ifdef DEBUG
		Serial.println("Please connect to the WiFi setup access point: ESP8266-Setup");
#endif
		//	drawProgress("请用手机设置本机WIFI", "SSID ESP8266-Setup");
	}
	else
	{
#ifdef DEBUG
		Serial.println("Scan WIFI");
#endif
		//	drawProgress("正在扫描WIFI...", "");
		int intPreferredWIFI = 0;
		WiFi.persistent(false);
		WiFi.mode(WIFI_STA);
		WiFi.disconnect();

		int n = WiFi.scanNetworks();
		if (n == 0)
		{
		}
		else
		{
			for (int i = 0; i < n; ++i)
			{
				for (int j = 0; j < numWIFIs; j++)
				{
					if (strcmp(WIFI_SSID[j], string2char(WiFi.SSID(i))) == 0)
					{
						intPreferredWIFI = j;
						break;
					}
				}
			}
		}
#ifdef DEBUG
		Serial.println("Connect WIFI");
		Serial.println(WIFI_SSID[intPreferredWIFI]);
#endif

		WiFi.persistent(true);
		WiFi.begin(WIFI_SSID[intPreferredWIFI], WIFI_PWD[intPreferredWIFI]);
		//	drawProgress("正在连接WIFI...", WIFI_SSID[intPreferredWIFI]);
		int WIFIcounter = intPreferredWIFI;
		while (WiFi.status() != WL_CONNECTED) {
			int counter = 0;
			while (counter < WIFI_TRY && WiFi.status() != WL_CONNECTED)
			{
				if (WiFi.status() == WL_CONNECTED) break;
				delay(1000);
				if (WiFi.status() == WL_CONNECTED) break;
				counter++;
			}
			if (WiFi.status() == WL_CONNECTED) break;
			WIFIcounter++;
			if (WIFIcounter >= numWIFIs) WIFIcounter = 0;
			WiFi.begin(WIFI_SSID[WIFIcounter], WIFI_PWD[WIFIcounter]);
			//		drawProgress("正在连接WIFI...", WIFI_SSID[WIFIcounter]);
			Serial.println(WIFI_SSID[WIFIcounter]);
		}
	}
}

String windDirectionTranslate(String stringInput) {
	String stringReturn = stringInput;
	stringReturn.replace("N", "北");
	stringReturn.replace("S", "南");
	stringReturn.replace("E", "东");
	stringReturn.replace("W", "西");
	stringReturn.replace("无持续", "无");
	return stringReturn;
}

void adjustBacklight(int(&lightLevel)[10], int BACKLIGHTPIN, int intBiasLevel, int intDynamicLevel) {
	// intBiasLevel is used to adjust dynamic range of the backlight vs light resistor
	// it should be between 50 and 200
	for (int i = 0; i < 9; ++i)
	{
		lightLevel[i] = lightLevel[i + 1]; // shift value forward
	}
	lightLevel[9] = analogRead(A0); // 0 very strong light, 200 and above dark, read new value
	int lightLevelSum = 0;
	for (int i = 0; i < 10; ++i)
	{
		lightLevelSum += lightLevel[i];
	}

	lightLevelSum = (lightLevelSum / 10 - 50 + intBiasLevel) * intDynamicLevel / 100;
#ifdef DEBUG
	Serial.print("Light Level: ");
	Serial.println(lightLevelSum);
#endif
	if (lightLevelSum < 10)
	{
		analogWrite(BACKLIGHTPIN, 1023); // Maximum is 1023
	}
	else if (lightLevelSum < 15)
	{
		analogWrite(BACKLIGHTPIN, 1000); // Maximum is 1023
	}
	else if (lightLevelSum < 20)
	{
		analogWrite(BACKLIGHTPIN, 900); // Maximum is 1023
	}
	else if (lightLevelSum < 25)
	{
		analogWrite(BACKLIGHTPIN, 800); // Maximum is 1023
	}
	else if (lightLevelSum < 30)
	{
		analogWrite(BACKLIGHTPIN, 700); // Maximum is 1023
	}
	else if (lightLevelSum < 35)
	{
		analogWrite(BACKLIGHTPIN, 600); // Maximum is 1023
	}
	else if (lightLevelSum < 40)
	{
		analogWrite(BACKLIGHTPIN, 500); // Maximum is 1023
	}
	else if (lightLevelSum < 45)
	{
		analogWrite(BACKLIGHTPIN, 400); // Maximum is 1023
	}
	else if (lightLevelSum < 50)
	{
		analogWrite(BACKLIGHTPIN, 300); // Maximum is 1023
	}
	else if (lightLevelSum < 60)
	{
		analogWrite(BACKLIGHTPIN, 200); // Maximum is 1023
	}
	else if (lightLevelSum < 75)
	{
		analogWrite(BACKLIGHTPIN, 100); // Maximum is 1023
	}
	else if (lightLevelSum < 100)
	{
		analogWrite(BACKLIGHTPIN, 75); // Maximum is 1023
	}
	else if (lightLevelSum < 150)
	{
		analogWrite(BACKLIGHTPIN, 50); // Maximum is 1023
	}
	else if (lightLevelSum < 250)
	{
		analogWrite(BACKLIGHTPIN, 25); // Maximum is 1023
	}
	else if (lightLevelSum < 400)
	{
		analogWrite(BACKLIGHTPIN, 12); // Maximum is 1023
	}
	else if (lightLevelSum < 600)
	{
		analogWrite(BACKLIGHTPIN, 6); // Maximum is 1023
	}
	else
	{
		analogWrite(BACKLIGHTPIN, 3); // Maximum is 1023
	}
}

void turnOffBacklight(int BACKLIGHTPIN, int minimumBackLight) {
	analogWrite(BACKLIGHTPIN, minimumBackLight); // Maximum is 1023
}

void selfTestBacklight(int BACKLIGHTPIN) {
	analogWrite(BACKLIGHTPIN, 5);
	delay(100);

	analogWrite(BACKLIGHTPIN, 10);
	delay(100);

	analogWrite(BACKLIGHTPIN, 20);
	delay(100);

	analogWrite(BACKLIGHTPIN, 50);
	delay(100);

	analogWrite(BACKLIGHTPIN, 100);
	delay(100);

	analogWrite(BACKLIGHTPIN, 200);
	delay(100);

	analogWrite(BACKLIGHTPIN, 400);
	delay(100);

	analogWrite(BACKLIGHTPIN, 600);
	delay(100);

	analogWrite(BACKLIGHTPIN, 800);
	delay(100);

	analogWrite(BACKLIGHTPIN, 1000);
	delay(100);

	analogWrite(BACKLIGHTPIN, 1023);
	delay(500);
}

String chooseMeteocon(String stringInput) {
	time_t nowTime = time(nullptr);
	struct tm* timeInfo;
	timeInfo = localtime(&nowTime);

	if (timeInfo->tm_hour > 6 && timeInfo->tm_hour < 18)
	{
		return stringInput.substring(0, 1);
	}
	else
	{
		return stringInput.substring(1, 2);
	}
}

void SMTPSend(String statusMessage, String recipient, String SMOKE_ALARM_LOCATION) {
	if (WiFi.status() != WL_CONNECTED) return;
	time_t nowTime = time(nullptr);
	struct tm* timeInfo;
	timeInfo = localtime(&nowTime);
	String completeDateTime = intToTwoDigitString(timeInfo->tm_hour) + ":" + intToTwoDigitString(timeInfo->tm_min) + ":" + intToTwoDigitString(timeInfo->tm_sec) + " " + String(timeInfo->tm_year + 1900) + "-" + intToTwoDigitString(timeInfo->tm_mon + 1) + "-" + intToTwoDigitString(timeInfo->tm_mday);
	String emailMessage = String("Subject: ") + String("Smoke Alarm ") + statusMessage + " " + completeDateTime + String(" At ") + SMOKE_ALARM_LOCATION + String("\r\n\r\n") + String("Smoke Alarm ") + statusMessage + " " + completeDateTime + String(" At ") + SMOKE_ALARM_LOCATION + String("\r\n\r\n");

#ifdef DEBUG
	Serial.println(emailMessage);
#endif

	if (SMTPSendMail(emailMessage, recipient) == 1)
	{
#ifdef DEBUG
		Serial.println("Email sent success!");
#endif
	}
	else
	{
#ifdef DEBUG
		Serial.println("Email sent failed!");
#endif
	}
}

// SECURITY NOTE: this sends SMTP AUTH LOGIN credentials over a plain, unencrypted
// WiFiClient connection (base64 is an encoding, not encryption - anyone on the network
// path can decode it trivially). Most SMTP providers today require STARTTLS/implicit TLS
// on port 587/465, which this simple hand-rolled client does not implement. Consider a
// library that supports SMTP over WiFiClientSecure (e.g. ESP-Mail-Client) instead of this
// function for anything beyond a private, trusted LAN with a mail server you control.
//
// Fill in your own server/credentials - never commit real ones. authLoginUser/authLoginPassword
// are the base64-encoded values SMTP AUTH LOGIN expects (base64 of the plain username/password).
const char SMTP_SERVER[] = "smtp.example.com";
const int SMTP_PORT = 587;
const char SMTP_AUTH_LOGIN_USER_BASE64[] = "BASE64_ENCODED_USERNAME";
const char SMTP_AUTH_LOGIN_PASSWORD_BASE64[] = "BASE64_ENCODED_PASSWORD";
const char SMTP_FROM_ADDRESS[] = "alerts@example.com";

byte SMTPSendMail(String message, String recipient)
{
	WiFiClient client;

	byte thisByte = 0;
	byte respCode;

#ifdef DEBUG
	Serial.println(F("connect to email server"));
#endif

	int retryCounter = 0;
	while (!client.connect(SMTP_SERVER, SMTP_PORT) == 1) {
#ifdef DEBUG
		Serial.println(".");
#endif
		delay(1000);
		retryCounter++;
		if (retryCounter > 10) {
#ifdef DEBUG
			Serial.println(F("connection failed"));
#endif
			return 0;
		}
	}
	if (!SMTPReceive(client)) return 0;
#ifdef DEBUG
	Serial.println(F("connected"));
	Serial.println(F("Sending hello"));
#endif
	client.println(String("EHLO ") + SMTP_SERVER);
	if (!SMTPReceive(client)) return 0;

#ifdef DEBUG
	Serial.println(F("Sending auth login"));
#endif
	client.println("auth login");
	if (!SMTPReceive(client)) return 0;

#ifdef DEBUG
	Serial.println(F("Sending User"));
#endif
	client.println(SMTP_AUTH_LOGIN_USER_BASE64);

	if (!SMTPReceive(client)) return 0;

#ifdef DEBUG
	Serial.println(F("Sending Password"));
#endif
	client.println(SMTP_AUTH_LOGIN_PASSWORD_BASE64);

	if (!SMTPReceive(client)) return 0;

#ifdef DEBUG
	Serial.println(F("Sending From"));
#endif
	client.println(String("MAIL From: <") + SMTP_FROM_ADDRESS + ">");
	if (!SMTPReceive(client)) return 0;

	// change to recipient address
#ifdef DEBUG
	Serial.println(F("Sending To"));
#endif
	client.println("RCPT To: <" + recipient + ">");
	if (!SMTPReceive(client)) return 0;

#ifdef DEBUG
	Serial.println(F("Sending DATA"));
#endif
	client.println("DATA");
	if (!SMTPReceive(client)) return 0;

#ifdef DEBUG
	Serial.println(F("Sending email"));
#endif

	client.println("To: Smoke Alarm <" + recipient + ">");
	client.println(String("From: Smoke Alarm <") + SMTP_FROM_ADDRESS + ">");
	client.println(message);
	client.println(".");
	if (!SMTPReceive(client)) return 0;
#ifdef DEBUG
	Serial.println(F("Sending QUIT"));
#endif
	client.println("QUIT");
	if (!SMTPReceive(client)) return 0;
	client.stop();
#ifdef DEBUG
	Serial.println(F("disconnected"));
#endif
	return 1;
}

byte SMTPReceive(WiFiClient client)
{
	byte respCode;
	byte thisByte;
	int loopCount = 0;

	while (!client.available()) {
		delay(1);
		loopCount++;

		// if nothing received for 10 seconds, timeout
		if (loopCount > 30000) {
			client.stop();
#ifdef DEBUG
			Serial.println(F("\r\nTimeout"));
#endif
			return 0;
		}
	}

	respCode = client.peek();
	while (client.available())
	{
		thisByte = client.read();
#ifdef DEBUG
		Serial.write(thisByte);
#endif
	}
	if (respCode >= '4')
	{
		SMTPFail(client);
		return 0;
	}
	return 1;
}


void SMTPFail(WiFiClient client)
{
	byte thisByte = 0;
	int loopCount = 0;
	client.println(F("QUIT"));
	while (!client.available()) {
		delay(1);
		loopCount++;
		// if nothing received for 10 seconds, timeout
		if (loopCount > 30000) {
			client.stop();
#ifdef DEBUG
			Serial.println(F("\r\nTimeout"));
#endif
			return;
		}
	}
	while (client.available())
	{
		thisByte = client.read();
#ifdef DEBUG
		Serial.write(thisByte);
#endif
	}
	client.stop();
#ifdef DEBUG
	Serial.println(F("disconnected"));
#endif
}

String intToTwoDigitString(int intInput) {
	if (intInput > 9)
	{
		return String(intInput);
	}
	else
	{
		return "0" + String(intInput);
	}
}

void writeBootWebSite(SettingsServerStruct *localSettingsServer, int currentSerial)
{
	if (WiFi.status() != WL_CONNECTED) return;
	String SettingsURL = localSettingsServer -> settingsBaseUrl + localSettingsServer -> settingsWriteBootUrl + String(currentSerial);
	String b = readHTTPWebSiteAsSring(localSettingsServer -> settingsServer, localSettingsServer -> settingsPort, SettingsURL);
#ifdef DEBUG
	Serial.println(b);
#endif
	return;
}

void writeDataWebSite(SettingsServerStruct *localSettingsServer, int currentSerial, int insideTemp, int insideHumidity, int outsideTemp, int outsideHumidity, int airQuality)
{
	if (WiFi.status() != WL_CONNECTED) return;
	String SettingsURL = localSettingsServer -> settingsBaseUrl + localSettingsServer -> settingsWriteDataUrl + String(currentSerial) + "&int=" + String(insideTemp) + "&inh=" + String(insideHumidity) + "&outt=" + String(outsideTemp) + "&outh=" + String(outsideHumidity) + "&air=" + String(airQuality);
	String b = readHTTPWebSiteAsSring(localSettingsServer -> settingsServer, localSettingsServer -> settingsPort, SettingsURL);
#ifdef DEBUG
	Serial.println(b);
#endif
	return;
}

String readHTTPWebSiteAsSring(String hostServer, int hostPort, String hostUrl)
{
	if (WiFi.status() != WL_CONNECTED) return "";
	WiFiClient client;
#ifdef DEBUG
	Serial.print(">> Connecting to ");
	Serial.println(hostServer);
#endif
	int retryCounter = 0;
	while (!client.connect(hostServer, hostPort))
	{
#ifdef DEBUG
		Serial.println(".");
#endif    delay(1000);
		retryCounter++;
		if (retryCounter > 10)
		{
			client.stop();
			return "";
		}
	}
#ifdef DEBUG
	Serial.print(">> Requesting URL: ");
	Serial.println(hostUrl);
	Serial.println("");
#endif
#ifdef DEBUG
	Serial.println(String("GET ") + hostUrl + " HTTP/1.0\r\n" + "Host: " + hostServer + USER_AGENT);
#endif
	client.print(String("GET ") + hostUrl + " HTTP/1.0\r\n" + "Host: " + hostServer + USER_AGENT);

	unsigned long timeout = millis();
	while (client.available() == 0) {
		if (millis() - timeout > 30000) {
#ifdef DEBUG
			Serial.println(">> Client Timeout !");
#endif
			client.stop();
			return "";
		}
	}
#ifdef DEBUG
	Serial.println("Web site connected reading data from: " + hostUrl);
#endif
	client.setTimeout(30000);
	int intTempCount = 0;
	String line = "";
	if (client.available())
	{
		line = client.readStringUntil('\n'); // Read first line of header which contains status code
	}
#ifdef DEBUG
	Serial.println("First Header: " + line);
#endif

	if (line.indexOf("1.1 200") > 0) // Correct status code
	{
#ifdef DEBUG
		Serial.println("Read the lines: ");
#endif
		bool contentBegins = false;
		String a;
		String b;
		while (client.available())
		{
			a= client.readString();// read the incoming data as string
			b += a;
		}
		b.replace("\r", "");
		int indexNumber = b.indexOf("\n\n");
		b = b.substring(indexNumber);
		b.trim();
#ifdef DEBUG
		Serial.println("Raw");
		Serial.println(b);
		Serial.println("Raw");
#endif
#ifdef DEBUG
		Serial.println();
		Serial.println("closing connection");
#endif
		client.stop();
		return b;
	}
	else
	{
#ifdef DEBUG
		Serial.println("Web site returned a wrong status code.");
#endif
		client.stop();
		return "";
	}
}

void readValueWebSite(SettingsServerStruct *localSettingsServer, int(&currentSerial), String(&Location), String(&Token), int(&Resistor), bool(&dummyMode), bool(&backlightOffMode), bool(&sendAlarmEmail), String(&alarmEmailAddress), int(&displayContrast), int(&displayMultiplier), int(&displayBias), int(&displayMinimumLevel), int(&displayMaximumLevel), int(&temperatureMultiplier), int(&temperatureBias), int(&humidityMultiplier), int(&humidityBias), int(&firmwareVersion), String(&firmwareBin))
{
	if (WiFi.status() != WL_CONNECTED) return;
	readMother(localSettingsServer);
	String currentMacAddress = WiFi.macAddress();
	String SettingsURL = localSettingsServer -> settingsBaseUrl + localSettingsServer -> settingsReadSettingUrl + currentMacAddress;
	String b = readHTTPWebSiteAsSring(localSettingsServer -> settingsServer, localSettingsServer -> settingsPort, SettingsURL);
	String c = "";
	int previousEndIndex = 0;
	int endIndex = -1;
	for (int intTempCount = 0; intTempCount <= 19; intTempCount++)
	{
#ifdef DEBUG
		Serial.println("");
		Serial.print("Line : ");
		Serial.println(intTempCount);
#endif
		endIndex = b.indexOf("\n", previousEndIndex + 1);
		String c;
		if (intTempCount==0)
		{
			c = b.substring(previousEndIndex, endIndex);
		}
		else
		{
			c = b.substring(previousEndIndex+1, endIndex);
		}
		c.replace("\n", "");
		c.trim();
#ifdef DEBUG
		Serial.print("Data: ");
		Serial.println(c);
#endif
		if (intTempCount == 0)
		{
			currentSerial = c.toInt();
		}
		else if (intTempCount == 1)
		{
			Location = c;
		}
		else if (intTempCount == 2)
		{
			Token = c;
		}
		else if (intTempCount == 3)
		{
			Resistor = c.toInt();
		}
		else if (intTempCount == 4)
		{
			displayContrast = c.toInt();
		}
		else if (intTempCount == 5)
		{
			displayBias = c.toInt();
		}
		else if (intTempCount == 6)
		{
			dummyMode = convertStringToBool(c);
		}
		else if (intTempCount == 7)
		{
			backlightOffMode = convertStringToBool(c);
		}
		else if (intTempCount == 8)
		{
			sendAlarmEmail = convertStringToBool(c);
		}
		else if (intTempCount == 9)
		{
			alarmEmailAddress = c;
		}
		else if (intTempCount == 10)
		{
			displayMultiplier = c.toInt();
		}
		else if (intTempCount == 11)
		{
			displayMinimumLevel = c.toInt();
		}
		else if (intTempCount == 12)
		{
			displayMaximumLevel = c.toInt();
		}
		else if (intTempCount == 13)
		{
			temperatureMultiplier = c.toInt();
		}
		else if (intTempCount == 14)
		{
			temperatureBias = c.toInt();
		}
		else if (intTempCount == 15)
		{
			humidityMultiplier = c.toInt();
		}
		else if (intTempCount == 16)
		{
			humidityBias = c.toInt();
		}
		else if (intTempCount == 18)
		{
			firmwareVersion = c.toInt();
		}
		else if (intTempCount == 19)
		{
			firmwareBin = c;
		}
		previousEndIndex = endIndex;

	}
	return;
}

bool convertStringToBool(String inputString)
{
	String tempString = inputString;
	tempString.trim();
	if (tempString == "1")
	{
		return true;
	}
	else
	{
		return false;
	}
}

void stopApp()
{
	delay(120000);
	ESP.restart();
}

void setMotherDefault(SettingsServerStruct *localSettingsServer)
{
	localSettingsServer -> settingsServer = SETTINGS_SERVER;
	localSettingsServer -> settingsPort = SETTINGS_PORT;
	localSettingsServer -> settingsBaseUrl = SETTINGS_BASE_URL;
	localSettingsServer -> settingsReadSettingUrl = SETTINGS_READ_SETTING_URL;
	localSettingsServer -> settingsReadTimerUrl = SETTINGS_READ_TIMER_URL;
	localSettingsServer -> settingsWriteBootUrl = SETTINGS_WRITE_BOOT_URL;
	localSettingsServer -> settingsWriteDataUrl = SETTINGS_WRITE_DATA_URL;
	localSettingsServer -> settingsOtaBinUrl = SETTINGS_OTA_BIN_URL;

}

void readMother(SettingsServerStruct *localSettingsServer)
{
	if (WiFi.status() != WL_CONNECTED) { setMotherDefault(localSettingsServer); return; }
	String b = readHTTPWebSiteAsSring(MOTHER_SERVER, MOTHER_PORT, MOTHER_FILE);
	String c = "";
	int previousEndIndex = 0;
	int endIndex = -1;
	for (int intTempCount = 0; intTempCount <= 7; intTempCount++)
	{
#ifdef DEBUG
		Serial.println("");
		Serial.print("Line : ");
		Serial.println(intTempCount);
#endif
		endIndex = b.indexOf("\n", previousEndIndex + 1);
		if (intTempCount==0)
		{
			c = b.substring(previousEndIndex, endIndex);
		}
		else
		{
			c = b.substring(previousEndIndex+1, endIndex);
		}
		c.replace("\n", "");
		c.trim();
#ifdef DEBUG
		Serial.print("Data: ");
		Serial.println(c);
#endif
			if (intTempCount == 0)
			{
				localSettingsServer -> settingsServer = c;
			}
			else if (intTempCount == 1)
			{
				localSettingsServer -> settingsPort = c.toInt();
			}
			else if (intTempCount == 2)
			{
				localSettingsServer -> settingsBaseUrl = c;
			}
			else if (intTempCount == 3)
			{
				localSettingsServer -> settingsReadSettingUrl = c;
			}
			else if (intTempCount == 4)
			{
				localSettingsServer -> settingsReadTimerUrl = c;
			}
			else if (intTempCount == 5)
			{
				localSettingsServer -> settingsWriteBootUrl = c;
			}
			else if (intTempCount == 6)
			{
				localSettingsServer -> settingsWriteDataUrl = c;
			}
			else if (intTempCount == 7)
			{
				localSettingsServer -> settingsOtaBinUrl = c;
			}
		previousEndIndex = endIndex;
	}
	return;
}
