#include <pgmspace.h>

#define SECRET
#define THINGNAME "lab_temperature-humidity"

const char WIFI_SSID[] = "UWNet";
const char WIFI_PASSWORD[] = "";
const char AWS_IOT_ENDPOINT[] = "";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
)EOF";

// Device certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
)KEY";