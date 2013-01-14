#ifndef _PACKETS_H_
#define _PACKETS_H_
#include <stdarg.h>

const uint32_t DIR_RIGHT    = 0;
const uint32_t DIR_UP       = 1;
const uint32_t DIR_LEFT     = 2;
const uint32_t DIR_DOWN     = 3;

const uint32_t MOVE_RIGHT   = 0;
const uint32_t MOVE_LEFT    = 1;
const uint32_t MOVE_FORWARD = 2;

const uint32_t PCKT_CONNECTION_REQUEST  = 1;
const uint32_t PCKT_CONNECTION_RESPONSE = 2;
const uint32_t PCKT_UPDATE              = 3;
const uint32_t PCKT_EVENT               = 4;

typedef struct Header {
    uint8_t type;              // PCKT_*
    uint32_t length;           // Tālākā satura garums (baitos)
} __attribute__((packed)) header_t;

/* Klienta - servera konekcijas nodibināšana */

typedef struct ConnectionRequest {
    /* Tukšums - Header.length = 0 */
} __attribute__((packed)) conn_req_t;

typedef struct ConnectionResponse {
    uint32_t id;               // Izsniegtais ID
    uint32_t height;           // Spēles laukuma augstums >= 30
    uint32_t width;            // Spēles laukuma platums >= 80
    uint32_t playerCount;      // Spēlētāju daudzums (vajadzīgs ?)
    uint32_t tailLength;       // Astes garums (vajadzīgs ?)
    uint32_t frameRate;        // Kadri sekundē
    uint32_t bulletSpeed;      // Lodes ātrums (kadros)
    uint32_t bulletCooldown;   // Lodes cooldowns (kadros)
    uint32_t timeout;          // ??
} __attribute__((packed)) conn_resp_t;

/* Servera updeiti */

typedef struct UpdatePlayerHeader {
    uint32_t playerCount;      // Cik UpdatePlayer struktūras sekos
} __attribute__((packed)) upd_player_header_t;

typedef struct UpdatePlayer {
    uint32_t id;
    uint32_t x;
    uint32_t y;
    uint32_t direction;        // DIR_*
    uint32_t cooldown;         // Lodes cooldown (Kadros)
    uint32_t gameover;         // 1 - miris, 0 - spēlē
} __attribute__((packed)) upd_player_t;

typedef struct UpdateBulletHeader {
    uint32_t bulletCount;      // Cik UpdateBullet struktūras sekos
} __attribute__((packed)) upd_bullet_header_t;

typedef struct UpdateBullet {
    uint32_t id;
    uint32_t x;
    uint32_t y;
    uint32_t direction;        // DIR_*
} __attribute__((packed)) upd_bullet_t;

typedef struct UpdateTotalTailHeader {
    uint32_t totalTailLength;  // Cik UpdateTailHeader struktūras sekos
} __attribute__((packed)) upd_total_tail_header_t;

typedef struct UpdateTailHeader {
    uint32_t id;
    uint32_t tailCount;        // Cik UpdateTail struktūras sekos
} __attribute__((packed)) upd_tail_header_t;

typedef struct UpdateTail {
    uint32_t x;
    uint32_t y;
} __attribute__((packed)) upd_tail_t;

/* Klienta notikumi */

typedef struct EventPlayer {
    uint8_t direction;         // MOVE_*
    uint8_t shot;              // 1 - izšauj, 0 - nešauj
} __attribute__((packed)) upd_event_t;

#endif /* _PAKCETS_H_  */
