#include "FirebaseHelper.h"
#include "../lib/secret.h"         // For API_KEY, DATABASE_URL, EMAIL, PASSWORD
#include <addons/TokenHelper.h>    // For tokenStatusCallback()

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void connectToFirebase()
{
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  auth.user.email = EMAIL;
  auth.user.password = PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}
