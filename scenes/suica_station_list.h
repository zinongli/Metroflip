#include <datetime.h>
#include <stdbool.h>
#include <furi.h>
#include "suica_structs_i.h"

#define RAILWAY_NUM 12

// Keikyu
static Station KeikyuAirport[6] = {
    {0x01, 11, "Keikyu Kamata"},
    {0x02, 12, "Kojiya"},
    {0x03, 13, "Otorii"},
    {0x04, 14, "Anamori-Inari"},
    {0x05, 15, "Tenkubashi"},
    {0x06, 16, "Haneda-Airport"},
};

static Station KeikyuMain[50] = {
    {0x01, 7, "Sengakuji"},
    {0x02, 1, "Shinagawa"},
    {0x03, 2, "Kita-Shinagawa"},
    {0x04, 3, "Shimbamba"},
    {0x06, 4, "Aomono-yokocho"},
    {0x07, 5, "Samezu"},
    {0x08, 6, "Tachiaigawa"},
    {0x09, 7, "Omorikaigan"},
    {0x0A, 8, "Heiwajima"},
    {0x0B, 9, "Omorimachi"},
    {0x0C, 10, "Umeyashiki"},
    {0x0D, 11, "Keikyu Kamata"},
    {0x0E, 18, "Zoshiki"},
    {0x0F, 19, "Rokugodote"},
    {0x10, 20, "Keikyu Kawasaki"},
    {0x11, 27, "Hatcho-nawate"},
    {0x12, 28, "Tsurumi-ichiba"},
    {0x13, 29, "Keikyu Tsurumi"},
    {0x14, 30, "Kagetsu-sojiji"},
    {0x15, 31, "Namamugi"},
    {0x16, 32, "Keikyu Shinkoyasu"},
    {0x17, 33, "Koyasu"},
    {0x18, 34, "Kanagawa-shimmachi"},
    {0x19, 35, "KK Higashi-kanagawa"},
    {0x1A, 36, "Kanagawa"},
    {0x1B, 37, "Yokohama"},
    {0x1C, 38, "Tobe"},
    {0x1D, 39, "Hinodecho"},
    {0x1E, 40, "Koganecho"},
    {0x1F, 41, "Minamiota"},
    {0x20, 42, "Idogaya"},
    {0x21, 43, "Gumyoji"},
    {0x22, 44, "Kamiooka"},
    {0x24, 45, "Byobugaura"},
    {0x25, 46, "Sugita"},
    {0x27, 47, "Keikyu Tomioka"},
    {0x28, 48, "Nokendai"},
    {0x29, 49, "Kanazawa-bunko"},
    {0x2A, 50, "Kanazawa-hakkei"},
    {0x2B, 54, "Oppama"},
    {0x2C, 55, "Keikyu Taura"},
    {0x2E, 56, "Anjinzuka"},
    {0x2F, 57, "Hemi"},
    {0x30, 58, "Shioiri"},
    {0x31, 59, "Yokosuka-chuo"},
    {0x32, 60, "Kenritsudaigaku"},
    {0x33, 61, "Horinouchi"},
    {0x34, 62, "Keikyu Otsu"},
    {0x35, 63, "Maborikaigan"},
    {0x36, 64, "Uraga"},
};

// Tokyo Metro
static Station TokyoMetroGinza[19] = {
    {0x2C, 19, "Asakusa"},       {0x2D, 18, "Tawaramachi"},   {0x2E, 17, "Inaricho"},
    {0x2F, 16, "Ueno"},          {0x30, 15, "Ueno-hirokoji"}, {0x31, 14, "Suehirocho"},
    {0x32, 13, "Kanda"},         {0x33, 12, "Mitsukoshimae"}, {0x34, 11, "Nihombashi"},
    {0x35, 10, "Kyobashi"},      {0x36, 9, "Ginza"},          {0x37, 8, "Shimbashi"},
    {0x38, 7, "Toranomon"},      {0x39, 6, "Tameike-sannō"},  {0x3A, 5, "Akasaka-mitsuke"},
    {0x3B, 4, "Aoyama-itchōme"}, {0x3C, 3, "Gaiemmae"},       {0x3D, 2, "Omote-sandō"},
    {0x3E, 1, "Shibuya"},
};

static Station TokyoMetroChiyoda[20] = {
    {0x5E, 1, "Yoyogi-uehara"},    {0x5D, 2, "Yoyogi-koen"},  {0x5C, 3, "Meiji-jingumae"},
    {0x5B, 4, "Omotesando"},       {0x59, 5, "Nogizaka"},     {0x58, 6, "Akasaka"},
    {0x57, 7, "Kokkai-gijidomae"}, {0x56, 8, "Kasumigaseki"}, {0x55, 9, "Hibiya"},
    {0x54, 10, "Nijubashimae"},    {0x53, 11, "Otemachi"},    {0x51, 12, "Shin-ochanomizu"},
    {0x50, 13, "Yushima"},         {0x4F, 14, "Nezu"},        {0x4E, 15, "Sendagi"},
    {0x4D, 16, "Nishi-Nippori"},   {0x4C, 17, "Machiya"},     {0x4A, 18, "Kita-senju"},
    {0x48, 19, "Ayase"},           {0x47, 20, "Kita-Ayase"},
};

static Station TokyoMetroYurakucho[24] = {
    {0x65, 1, "Wakoshi"},
    {0x66, 2, "Chikatetsu-narimasu"},
    {0x68, 3, "Chikatetsu-akatsuka"},
    {0x6A, 4, "Heiwadai"},
    {0x6B, 5, "Hikawadai"},
    {0x6D, 6, "Kotake-mukaihara"},
    {0x6E, 7, "Senkawa"},
    {0x6F, 8, "Kanamecho"},
    {0x70, 9, "Ikebukuro"},
    {0x71, 10, "Higashi-ikebukuro"},
    {0x72, 11, "Gokokuji"},
    {0x73, 12, "Edogawabashi"},
    {0x75, 13, "Iidabashi"},
    {0x76, 14, "Ichigaya"},
    {0x77, 15, "Kojimachi"},
    {0x78, 16, "Nagatacho"},
    {0x79, 17, "Sakuradamon"},
    {0x7A, 18, "Yurakucho"},
    {0x7B, 19, "Ginza-itchome"},
    {0x7C, 20, "Shintomicho"},
    {0x7D, 21, "Tsukishima"},
    {0x7E, 22, "Toyosu"},
    {0x7F, 23, "Tatsumi"},
    {0x80, 24, "Shin-kiba"},
};

static Station TokyoMetroHibiya[21] = {
    {0x25, 22, "Kita-senju"},    {0x27, 21, "Minami-senju"}, {0x28, 20, "Minowa"},
    {0x29, 19, "Iriya"},         {0x2A, 18, "Ueno"},         {0x2B, 17, "Naka-okachimachi"},
    {0x2C, 16, "Akihabara"},     {0x2D, 15, "Kodenmacho"},   {0x2E, 14, "Ningyocho"},
    {0x2F, 13, "Kayabacho"},     {0x30, 12, "Hatchobori"},   {0x31, 11, "Tsukiji"},
    {0x32, 10, "Higashi-ginza"}, {0x33, 9, "Ginza"},         {0x34, 8, "Hibiya"},
    {0x35, 7, "Kasumigaseki"},   {0x37, 5, "Kamiyacho"},     {0x39, 4, "Roppongi"},
    {0x3B, 3, "Hiro-o"},         {0x3D, 2, "Ebisu"},         {0x3E, 1, "Naka-meguro"},
};

static Station TokyoMetroTozai[23] = {
    {0x41, 1, "Nakano"},
    {0x43, 2, "Ochiai"},
    {0x45, 3, "Takadanobaba"},
    {0x47, 4, "Waseda"},
    {0x48, 5, "Kagurazaka"},
    {0x49, 6, "Iidabashi"},
    {0x4A, 7, "Kudanshita"},
    {0x4B, 8, "Takebashi"},
    {0x4C, 9, "Otemachi"},
    {0x4D, 10, "Nihombashi"},
    {0x4E, 11, "Kayabacho"},
    {0x50, 12, "Monzen-nakacho"},
    {0x51, 13, "Kiba"},
    {0x52, 14, "Toyocho"},
    {0x53, 15, "Minami-sunamachi"},
    {0x54, 16, "Nishi-kasai"},
    {0x55, 17, "Kasai"},
    {0x57, 18, "Urayasu"},
    {0x58, 19, "Minami-gyotoku"},
    {0x59, 20, "Gyotoku"},
    {0x5A, 21, "Myoden"},
    {0x5C, 22, "Baraki-nakayama"},
    {0x5E, 23, "Nishi-funabashi"},
};

static Station TokyoMetroMarunouchi[25] = {
    {0x21, 25, "Ikebukuro"},
    {0x23, 24, "Shin-otsuka"},
    {0x24, 23, "Myogadani"},
    {0x26, 22, "Korakuen"},
    {0x27, 21, "Hongo-sanchome"},
    {0x28, 20, "Ochanomizu"},
    {0x29, 19, "Awajicho"},
    {0x2A, 18, "Otemachi"},
    {0x2B, 17, "Tokyo"},
    {0x2C, 16, "Ginza"},
    {0x2D, 15, "Kasumigaseki"},
    {0x2E, 14, "Kokkai-gijidomae"},
    {0x2F, 13, "Akasaka-mitsuke"},
    {0x30, 12, "Yotsuya"},
    {0x31, 11, "Yotsuya-sanchome"},
    {0x32, 10, "Shinjuku-gyoemmae"},
    {0x33, 9, "Shinjuku-sanchome"},
    {0x34, 8, "Shinjuku"},
    {0x35, 7, "Nishi-shinjuku"},
    {0x36, 6, "Nakano-sakaue"},
    {0x37, 5, "Shin-nakano"},
    {0x38, 4, "Higashi-koenji"},
    {0x39, 3, "Shin-koenji"},
    {0x3A, 2, "Minami-asagaya"},
    {0x3C, 1, "Ogikubo"},
};

static Station TokyoMetroMarunouchiHonancho[4] = {
    {0x41, 6, "Nakano-sakaue"},
    {0x42, 5, "Nakano-shimbashi"},
    {0x43, 4, "Nakano-fujimicho"},
    {0x44, 3, "Honancho"},
};

static Station TokyoMetroHanzomon[14] = {
    {0x21, 1, "Shibuya"},
    {0x22, 2, "Omotesando"},
    {0x23, 3, "Aoyama-itchome"},
    {0x26, 4, "Nagatacho"},
    {0x27, 5, "Hanzomon"},
    {0x28, 6, "Kudanshita"},
    {0x29, 7, "Jimbocho"},
    {0x2B, 8, "Otemachi"},
    {0x2C, 9, "Mitsukoshimae"},
    {0x2D, 10, "Suitengumae"},
    {0x2F, 11, "Kiyosumi-shirakawa"},
    {0x31, 12, "Sumiyoshi"},
    {0x33, 13, "Kinshicho"},
    {0x35, 14, "Oshiage"},
};

static Station TokyoMetroNamboku[19] = {
    {0x24, 1, "Meguro"},
    {0x25, 2, "Shirokanedai"},
    {0x26, 3, "Shirokane-takanawa"},
    {0x28, 4, "Azabu-juban"},
    {0x29, 5, "Roppongi-itchome"},
    {0x2A, 6, "Tameike-sanno"},
    {0x2C, 7, "Nagatacho"},
    {0x2E, 8, "Yotsuya"},
    {0x30, 9, "Ichigaya"},
    {0x32, 10, "Iidabashi"},
    {0x33, 11, "Korakuen"},
    {0x34, 12, "Todaimae"},
    {0x35, 13, "Hon-komagome"},
    {0x36, 14, "Komagome"},
    {0x37, 15, "Nishigahara"},
    {0x38, 16, "Oji"},
    {0x39, 17, "Oji-kamiya"},
    {0x3B, 18, "Shimo"},
    {0x3C, 19, "Akabane-iwabuchi"},
};

// Toei
static Station ToeiAsakusa[20] = {
    {0x01, 20, "Oshiage"},       {0x02, 19, "Honjo-azumabashi"}, {0x03, 18, "Asakusa"},
    {0x04, 17, "Kuramae"},       {0x05, 16, "Asakusabashi"},     {0x06, 15, "Higashi-nihombashi"},
    {0x07, 14, "Ningyocho"},     {0x08, 13, "Nihombashi"},       {0x09, 12, "Takaracho"},
    {0x0A, 11, "Higashi-ginza"}, {0x0B, 10, "Shimbashi"},        {0x0C, 9, "Daimon"},
    {0x0D, 8, "Mita"},           {0x0E, 7, "Sengakuji"},         {0x0F, 6, "Takanawadai"},
    {0x10, 5, "Gotanda"},        {0x11, 4, "Togoshi"},           {0x12, 3, "Nakanobu"},
    {0x13, 2, "Magome"},         {0x14, 1, "Nishi-magome"},
};

static Station UnknownLine[1] = {
    {0x00, 0, "Unknown"},
};
