#include <datetime.h>
#include <stdbool.h>
#include <furi.h>
#include "suica_structs_i.h"

#define RAILWAY_NUM 20 // Don't count Unknown

// Japan Railway East 2 lines
static Station JRETokaidoMainLine[] = {
    {0x01, 1, "Tokyo", "TYO"},            
    {0x03, 2, "Shimbashi", "SMB"},        
    {0x07, 3, "Shinagawa", "SGW"},        
    {0x0D, 4, "Kawasaki", "KWS"},        
    {0x12, 5, "Yokohama", "YHM"},        
    {0x16, 6, "Totsuka", "TTK"},         
    {0x18, 7, "Ofuna", "OFN"},           
    {0x19, 8, "Fujisawa", 0},        
    {0x1A, 9, "Tsujido", 0},         
    {0x1B, 10, "Chigasaki", 0},       
    {0x1D, 11, "Hiratsuka", 0},       
    {0x1E, 12, "Oiso", 0},            
    {0x20, 13, "Ninomiya", 0},        
    {0x21, 14, "Kozu", 0},            
    {0x22, 15, "Kamonomiya", 0},      
    {0x23, 16, "Odawara", 0},         
    {0x25, 17, "Hayakawa", 0},        
    {0x26, 18, "Nebukawa", 0},        
    {0x28, 19, "Manazuru", 0},        
    {0x29, 20, "Yugawara", 0},        
    {0x2B, 21, "Atami", 0},           
};

static Station JREKeihinTohoku[] = {
    {0x01, 26, "Tokyo", "TYO"},            
    {0x02, 25, "Yurakucho", 0},        
    {0x03, 24, "Shimbashi", "SMB"},        
    {0x04, 23, "Hamamatsucho", "HMC"},     
    {0x06, 22, "Tamachi", 0},     
    // Missing Takanawa Gateway     
    {0x07, 20, "Shinagawa", "SGW"},        
    {0x08, 19, "Oimachi", 0},          
    {0x09, 18, "Omori", 0},            
    {0x0B, 17, "Kamata", 0},           
    {0x0D, 16, "Kawasaki", "KWS"},        
    {0x0E, 15, "Tsurumi", 0},         
    {0x10, 14, "Shin-koyasu", 0},     
    {0x11, 13, "Higashi-kanagawa", 0},
    {0x12, 12, "Yokohama", "YHM"},        
};

static Station JRENegishi[] = {
    {0x04, 2, "Hongodai", 0},     
    {0x06, 3, "Konandai", 0},     
    {0x07, 4, "Yokodai", 0},       
    {0x08, 5, "Shin-sugita", 0},   
    {0x09, 6, "Isogo", 0},         
    {0x0A, 7, "Negishi", 0},       
    {0x0C, 8, "Yamate", 0},        
    {0x0D, 9, "Ishikawacho", 0},  
    {0x0E, 10, "Kannai", 0},       
    {0x0F, 11, "Sakuragicho", 0},  
};

// Tokyo Waterfront Area Rapid Transit
static Station TokyoRinkaiLine[] = {
    {0x01, 1, "Shin-kiba", 0},          // 1 decimal -> 0x01 hex
    {0x03, 2, "Shinonome", 0},         // 3 decimal -> 0x03 hex
    {0x04, 3, "Kokusai-tenjijo", 0},   // 4 decimal -> 0x04 hex
    {0x05, 4, "Tokyo Teleport", 0},    // 5 decimal -> 0x05 hex
    {0x06, 5, "Tennozu Isle", 0},      // 6 decimal -> 0x06 hex
    {0x07, 6, "Shinagawa Seaside", 0}, // 7 decimal -> 0x07 hex
    {0x08, 7, "Oimachi", 0},           // 8 decimal -> 0x08 hex
    {0x0A, 8, "Osaki", 0},             // 10 decimal -> 0x0A hex
};


// Keikyu 2 lines
static Station KeikyuAirport[6] = {
    {0x01, 11, "Keikyu Kamata", 0},
    {0x02, 12, "Kojiya", 0},
    {0x03, 13, "Otorii", 0},
    {0x04, 14, "Anamori-Inari", 0},
    {0x05, 15, "Tenkubashi", 0},
    {0x06, 16, "Haneda-Airport", 0},
};

static Station KeikyuMain[50] = {
    {0x01, 7, "Sengakuji", 0},
    {0x02, 1, "Shinagawa", 0},
    {0x03, 2, "Kita-Shinagawa", 0},
    {0x04, 3, "Shimbamba", 0},
    {0x06, 4, "Aomono-yokocho", 0},
    {0x07, 5, "Samezu", 0},
    {0x08, 6, "Tachiaigawa", 0},
    {0x09, 7, "Omorikaigan", 0},
    {0x0A, 8, "Heiwajima", 0},
    {0x0B, 9, "Omorimachi", 0},
    {0x0C, 10, "Umeyashiki", 0},
    {0x0D, 11, "Keikyu Kamata", 0},
    {0x0E, 18, "Zoshiki", 0},
    {0x0F, 19, "Rokugodote", 0},
    {0x10, 20, "Keikyu Kawasaki", 0},
    {0x11, 27, "Hatcho-nawate", 0},
    {0x12, 28, "Tsurumi-ichiba", 0},
    {0x13, 29, "Keikyu Tsurumi", 0},
    {0x14, 30, "Kagetsu-sojiji", 0},
    {0x15, 31, "Namamugi", 0},
    {0x16, 32, "Keikyu Shinkoyasu", 0},
    {0x17, 33, "Koyasu", 0},
    {0x18, 34, "Kanagawa-shimmachi", 0},
    {0x19, 35, "KK Higashi-kanagawa", 0},
    {0x1A, 36, "Kanagawa", 0},
    {0x1B, 37, "Yokohama", 0},
    {0x1C, 38, "Tobe", 0},
    {0x1D, 39, "Hinodecho", 0},
    {0x1E, 40, "Koganecho", 0},
    {0x1F, 41, "Minamiota", 0},
    {0x20, 42, "Idogaya", 0},
    {0x21, 43, "Gumyoji", 0},
    {0x22, 44, "Kamiooka", 0},
    {0x24, 45, "Byobugaura", 0},
    {0x25, 46, "Sugita", 0},
    {0x27, 47, "Keikyu Tomioka", 0},
    {0x28, 48, "Nokendai", 0},
    {0x29, 49, "Kanazawa-bunko", 0},
    {0x2A, 50, "Kanazawa-hakkei", 0},
    {0x2B, 54, "Oppama", 0},
    {0x2C, 55, "Keikyu Taura", 0},
    {0x2E, 56, "Anjinzuka", 0},
    {0x2F, 57, "Hemi", 0},
    {0x30, 58, "Shioiri", 0},
    {0x31, 59, "Yokosuka-chuo", 0},
    {0x32, 60, "Kenritsudaigaku", 0},
    {0x33, 61, "Horinouchi", 0},
    {0x34, 62, "Keikyu Otsu", 0},
    {0x35, 63, "Maborikaigan", 0},
    {0x36, 64, "Uraga", 0},
};

// Tokyo Metro 9 lines
static Station TokyoMetroGinza[19] = {
    {0x2C, 19, "Asakusa", 0},       {0x2D, 18, "Tawaramachi", 0},   {0x2E, 17, "Inaricho", 0},
    {0x2F, 16, "Ueno", 0},          {0x30, 15, "Ueno-hirokoji", 0}, {0x31, 14, "Suehirocho", 0},
    {0x32, 13, "Kanda", 0},         {0x33, 12, "Mitsukoshimae", 0}, {0x34, 11, "Nihombashi", 0},
    {0x35, 10, "Kyobashi", 0},      {0x36, 9, "Ginza", 0},          {0x37, 8, "Shimbashi", 0},
    {0x38, 7, "Toranomon", 0},      {0x39, 6, "Tameike-sannō", 0},  {0x3A, 5, "Akasaka-mitsuke", 0},
    {0x3B, 4, "Aoyama-itchōme", 0}, {0x3C, 3, "Gaiemmae", 0},       {0x3D, 2, "Omote-sandō", 0},
    {0x3E, 1, "Shibuya", 0},
};

static Station TokyoMetroChiyoda[20] = {
    {0x5E, 1, "Yoyogi-uehara", 0},    {0x5D, 2, "Yoyogi-koen", 0},  {0x5C, 3, "Meiji-jingumae", 0},
    {0x5B, 4, "Omotesando", 0},       {0x59, 5, "Nogizaka", 0},     {0x58, 6, "Akasaka", 0},
    {0x57, 7, "Kokkai-gijidomae", 0}, {0x56, 8, "Kasumigaseki", 0}, {0x55, 9, "Hibiya", 0},
    {0x54, 10, "Nijubashimae", 0},    {0x53, 11, "Otemachi", 0},    {0x51, 12, "Shin-ochanomizu", 0},
    {0x50, 13, "Yushima", 0},         {0x4F, 14, "Nezu", 0},        {0x4E, 15, "Sendagi", 0},
    {0x4D, 16, "Nishi-Nippori", 0},   {0x4C, 17, "Machiya", 0},     {0x4A, 18, "Kita-senju", 0},
    {0x48, 19, "Ayase", 0},           {0x47, 20, "Kita-Ayase", 0},
};

static Station TokyoMetroYurakucho[24] = {
    {0x65, 1, "Wakoshi", 0},
    {0x66, 2, "Chikatetsu-narimasu", 0},
    {0x68, 3, "Chikatetsu-akatsuka", 0},
    {0x6A, 4, "Heiwadai", 0},
    {0x6B, 5, "Hikawadai", 0},
    {0x6D, 6, "Kotake-mukaihara", 0},
    {0x6E, 7, "Senkawa", 0},
    {0x6F, 8, "Kanamecho", 0},
    {0x70, 9, "Ikebukuro", 0},
    {0x71, 10, "Higashi-ikebukuro", 0},
    {0x72, 11, "Gokokuji", 0},
    {0x73, 12, "Edogawabashi", 0},
    {0x75, 13, "Iidabashi", 0},
    {0x76, 14, "Ichigaya", 0},
    {0x77, 15, "Kojimachi", 0},
    {0x78, 16, "Nagatacho", 0},
    {0x79, 17, "Sakuradamon", 0},
    {0x7A, 18, "Yurakucho", 0},
    {0x7B, 19, "Ginza-itchome", 0},
    {0x7C, 20, "Shintomicho", 0},
    {0x7D, 21, "Tsukishima", 0},
    {0x7E, 22, "Toyosu", 0},
    {0x7F, 23, "Tatsumi", 0},
    {0x80, 24, "Shin-kiba", 0},
};

static Station TokyoMetroHibiya[21] = {
    {0x25, 22, "Kita-senju", 0},    {0x27, 21, "Minami-senju", 0}, {0x28, 20, "Minowa", 0},
    {0x29, 19, "Iriya", 0},         {0x2A, 18, "Ueno", 0},         {0x2B, 17, "Naka-okachimachi", 0},
    {0x2C, 16, "Akihabara", 0},     {0x2D, 15, "Kodenmacho", 0},   {0x2E, 14, "Ningyocho", 0},
    {0x2F, 13, "Kayabacho", 0},     {0x30, 12, "Hatchobori", 0},   {0x31, 11, "Tsukiji", 0},
    {0x32, 10, "Higashi-ginza", 0}, {0x33, 9, "Ginza", 0},         {0x34, 8, "Hibiya", 0},
    {0x35, 7, "Kasumigaseki", 0},   {0x37, 5, "Kamiyacho", 0},     {0x39, 4, "Roppongi", 0},
    {0x3B, 3, "Hiro-o", 0},         {0x3D, 2, "Ebisu", 0},         {0x3E, 1, "Naka-meguro", 0},
};

static Station TokyoMetroTozai[23] = {
    {0x41, 1, "Nakano", 0},
    {0x43, 2, "Ochiai", 0},
    {0x45, 3, "Takadanobaba", 0},
    {0x47, 4, "Waseda", 0},
    {0x48, 5, "Kagurazaka", 0},
    {0x49, 6, "Iidabashi", 0},
    {0x4A, 7, "Kudanshita", 0},
    {0x4B, 8, "Takebashi", 0},
    {0x4C, 9, "Otemachi", 0},
    {0x4D, 10, "Nihombashi", 0},
    {0x4E, 11, "Kayabacho", 0},
    {0x50, 12, "Monzen-nakacho", 0},
    {0x51, 13, "Kiba", 0},
    {0x52, 14, "Toyocho", 0},
    {0x53, 15, "Minami-sunamachi", 0},
    {0x54, 16, "Nishi-kasai", 0},
    {0x55, 17, "Kasai", 0},
    {0x57, 18, "Urayasu", 0},
    {0x58, 19, "Minami-gyotoku", 0},
    {0x59, 20, "Gyotoku", 0},
    {0x5A, 21, "Myoden", 0},
    {0x5C, 22, "Baraki-nakayama", 0},
    {0x5E, 23, "Nishi-funabashi", 0},
};

static Station TokyoMetroMarunouchi[25] = {
    {0x21, 25, "Ikebukuro", 0},
    {0x23, 24, "Shin-otsuka", 0},
    {0x24, 23, "Myogadani", 0},
    {0x26, 22, "Korakuen", 0},
    {0x27, 21, "Hongo-sanchome", 0},
    {0x28, 20, "Ochanomizu", 0},
    {0x29, 19, "Awajicho", 0},
    {0x2A, 18, "Otemachi", 0},
    {0x2B, 17, "Tokyo", 0},
    {0x2C, 16, "Ginza", 0},
    {0x2D, 15, "Kasumigaseki", 0},
    {0x2E, 14, "Kokkai-gijidomae", 0},
    {0x2F, 13, "Akasaka-mitsuke", 0},
    {0x30, 12, "Yotsuya", 0},
    {0x31, 11, "Yotsuya-sanchome", 0},
    {0x32, 10, "Shinjuku-gyoemmae", 0},
    {0x33, 9, "Shinjuku-sanchome", 0},
    {0x34, 8, "Shinjuku", 0},
    {0x35, 7, "Nishi-shinjuku", 0},
    {0x36, 6, "Nakano-sakaue", 0},
    {0x37, 5, "Shin-nakano", 0},
    {0x38, 4, "Higashi-koenji", 0},
    {0x39, 3, "Shin-koenji", 0},
    {0x3A, 2, "Minami-asagaya", 0},
    {0x3C, 1, "Ogikubo", 0},
};

static Station TokyoMetroMarunouchiHonancho[4] = {
    {0x41, 6, "Nakano-sakaue", 0},
    {0x42, 5, "Nakano-shimbashi", 0},
    {0x43, 4, "Nakano-fujimicho", 0},
    {0x44, 3, "Honancho", 0},
};

static Station TokyoMetroHanzomon[14] = {
    {0x21, 1, "Shibuya", 0},
    {0x22, 2, "Omotesando", 0},
    {0x23, 3, "Aoyama-itchome", 0},
    {0x26, 4, "Nagatacho", 0},
    {0x27, 5, "Hanzomon", 0},
    {0x28, 6, "Kudanshita", 0},
    {0x29, 7, "Jimbocho", 0},
    {0x2B, 8, "Otemachi", 0},
    {0x2C, 9, "Mitsukoshimae", 0},
    {0x2D, 10, "Suitengumae", 0},
    {0x2F, 11, "Kiyosumi-shirakawa", 0},
    {0x31, 12, "Sumiyoshi", 0},
    {0x33, 13, "Kinshicho", 0},
    {0x35, 14, "Oshiage", 0},
};

static Station TokyoMetroNamboku[19] = {
    {0x24, 1, "Meguro", 0},
    {0x25, 2, "Shirokanedai", 0},
    {0x26, 3, "Shirokane-takanawa", 0},
    {0x28, 4, "Azabu-juban", 0},
    {0x29, 5, "Roppongi-itchome", 0},
    {0x2A, 6, "Tameike-sanno", 0},
    {0x2C, 7, "Nagatacho", 0},
    {0x2E, 8, "Yotsuya", 0},
    {0x30, 9, "Ichigaya", 0},
    {0x32, 10, "Iidabashi", 0},
    {0x33, 11, "Korakuen", 0},
    {0x34, 12, "Todaimae", 0},
    {0x35, 13, "Hon-komagome", 0},
    {0x36, 14, "Komagome", 0},
    {0x37, 15, "Nishigahara", 0},
    {0x38, 16, "Oji", 0},
    {0x39, 17, "Oji-kamiya", 0},
    {0x3B, 18, "Shimo", 0},
    {0x3C, 19, "Akabane-iwabuchi", 0},
};

// Toei 5 lines (double Oedo)
static Station ToeiAsakusa[20] = {
    {0x01, 20, "Oshiage", 0},       {0x02, 19, "Honjo-azumabashi", 0}, {0x03, 18, "Asakusa", 0},
    {0x04, 17, "Kuramae", 0},       {0x05, 16, "Asakusabashi", 0},     {0x06, 15, "Higashi-nihombashi", 0},
    {0x07, 14, "Ningyocho", 0},     {0x08, 13, "Nihombashi", 0},       {0x09, 12, "Takaracho", 0},
    {0x0A, 11, "Higashi-ginza", 0}, {0x0B, 10, "Shimbashi", 0},        {0x0C, 9, "Daimon", 0},
    {0x0D, 8, "Mita", 0},           {0x0E, 7, "Sengakuji", 0},         {0x0F, 6, "Takanawadai", 0},
    {0x10, 5, "Gotanda", 0},        {0x11, 4, "Togoshi", 0},           {0x12, 3, "Nakanobu", 0},
    {0x13, 2, "Magome", 0},         {0x14, 1, "Nishi-magome", 0},
};

static Station ToeiMita[27] = {
    {0x27, 27, "Nishi-takashimadaira", 0}, 
    {0x28, 26, "Shin-takashimadaira", 0},  
    {0x29, 25, "Takashimadaira", 0},       
    {0x2A, 24, "Nishidai", 0},             
    {0x2B, 23, "Hasune", 0},               
    {0x2C, 22, "Shimura-sanchome", 0},     
    {0x2D, 21, "Shimura-sakaue", 0},       
    {0x2E, 20, "Motohasunuma", 0},         
    {0x2F, 19, "Itabashi-honcho", 0},      
    {0x30, 18, "Itabashikuyakushomae", 0},
    {0x31, 17, "Shin-itabashi", 0},        
    {0x32, 16, "Nishi-sugamo", 0},         
    {0x33, 15, "Sugamo", 0},               
    {0x34, 14, "Sengoku", 0},              
    {0x35, 13, "Hakusan", 0},              
    {0x36, 12, "Kasuga", 0},               
    {0x37, 11, "Suidobashi", 0},           
    {0x38, 10, "Jimbocho", 0},             
    {0x39, 9,  "Otemachi", 0},             
    {0x3A, 8,  "Hibiya", 0},               
    {0x3B, 7,  "Uchisaiwaicho", 0},        
    {0x3C, 6,  "Onarimon", 0},             
    {0x3D, 5,  "Shibakoen", 0},            
    {0x3E, 4,  "Mita", 0},                 
    {0x40, 3,  "Shirokane-takanawa", 0},   
    {0x42, 2,  "Shirokanedai", 0},         
    {0x44, 1,  "Meguro", 0},               
};

static Station ToeiShinjuku[21] = {
    {0x01, 1, "Shinjuku", 0},           
    {0x02, 2, "Shinjuku-sanchome", 0},  
    {0x03, 3, "Akebonobashi", 0},       
    {0x04, 4, "Ichigaya", 0},           
    {0x05, 5, "Kudanshita", 0},         
    {0x06, 6, "Jimbocho", 0},           
    {0x07, 7, "Ogawamachi", 0},         
    {0x08, 8, "Iwamotocho", 0},         
    {0x09, 9, "Bakuro-yokoyama", 0},    
    {0x0A, 10, "Hamacho", 0},           
    {0x0B, 11, "Morishita", 0},         
    {0x0C, 12, "Kikukawa", 0},          
    {0x0D, 13, "Sumiyoshi", 0},         
    {0x0E, 14, "Nishi-ojima", 0},       
    {0x0F, 15, "Ojima", 0},             
    {0x11, 16, "Higashi-ojima", 0},     
    {0x13, 17, "Funabori", 0},          
    {0x15, 18, "Ichinoe", 0},           
    {0x17, 19, "Mizue", 0},             
    {0x19, 20, "Shinozaki", 0},         
    {0x1B, 21, "Motoyawata", 0},        
};

static Station ToeiOedoPartA[26] = {
    {0x04, 13, "Morishita", 0},          
    {0x06, 14, "Kiyosumi-shirakawa", 0}, 
    {0x08, 15, "Monzen-nakacho", 0},     
    {0x0A, 16, "Tsukishima", 0},         
    {0x0C, 17, "Kachidoki", 0},          
    {0x0E, 18, "Tsukiji-shijo", 0},      
    {0x10, 19, "Shiodome", 0},           
    {0x12, 20, "Daimon", 0},             
    {0x14, 21, "Akabanebashi", 0},       
    {0x16, 22, "Azabu-juban", 0},        
    {0x18, 23, "Roppongi", 0},           
    {0x1A, 24, "Aoyama-itchome", 0},     
    {0x1C, 25, "Kokuritsu-kyogijo", 0},  
    {0x1E, 26, "Yoyogi", 0},             
    {0x20, 27, "Shinjuku", 0},           
    {0x22, 28, "Tochomae", 0},           
    {0x24, 29, "Nishi-shinjuku-gochome", 0},
    {0x26, 30, "Nakano-sakaue", 0},      
    {0x28, 31, "Higashi-nakano", 0},      
    {0x2A, 32, "Nakai", 0},               
    {0x2C, 33, "Ochiai-minami-nagasaki", 0},
    {0x2E, 34, "Shin-egota", 0},          
    {0x30, 35, "Nerima", 0},              
    {0x32, 36, "Toshimaen", 0},           
    {0x34, 37, "Nerima-kasugacho", 0},    
    {0x36, 38, "Hikarigaoka", 0},       
};

static Station ToeiOedoPartB[14] = {
    {0x03, 13, "Morishita", 0},          
    {0x05, 12, "Ryogoku", 0},            
    {0x07, 11, "Kuramae", 0},            
    {0x09, 10, "Shin-okachimachi", 0},   
    {0x0B, 9, "Ueno-okachimachi", 0},   
    {0x0D, 8, "Hongo-sanchome", 0},     
    {0x0F, 7, "Kasuga", 0},             
    {0x11, 6, "Iidabashi", 0},          
    {0x13, 5, "Ushigome-kagurazaka", 0},
    {0x15, 4, "Ushigome-yanagicho", 0},
    {0x17, 3, "Wakamatsu-kawada", 0},  
    {0x19, 2, "Higashi-shinjuku", 0},  
    {0x1B, 1, "Shinjuku-nishiguchi", 0},
    {0x1D, 28, "Tochomae", 0},          
};


// Unknown
static Station UnknownLine[1] = {
    {0x00, 0, "Unknown", 0},
};
