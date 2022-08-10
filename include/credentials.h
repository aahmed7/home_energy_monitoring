// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use:
// InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "https://eu-central-1-1.aws.cloud2.influxdata.com"
// InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data ->
// Tokens -> <select token>)
#define INFLUXDB_TOKEN \
    ""
// InfluxDB v2 organization id (Use: InfluxDB UI -> User -> About -> Common Ids )
#define INFLUXDB_ORG ""
// InfluxDB v2 bucket name (Use: InfluxDB UI ->  Data -> Buckets)
#define INFLUXDB_BUCKET ""

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
#define TZ_INFO ""
// NTP servers the for time synchronization.
// For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nis.gov"

// Wifi credentials.
const char* ssid = "";
const char* password = "";