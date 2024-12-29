#include "../metroflip_i.h"
#include "../api/calypso/calypso_util.h"
#include "../api/calypso/cards/navigo.h"
#include <datetime.h>
#include <stdbool.h>

#ifndef METRO_LIST_H
#define METRO_LIST_H

#ifndef NAVIGO_H
#define NAVIGO_H

void metroflip_back_button_widget_callback(GuiButtonType result, InputType type, void* context);
void metroflip_next_button_widget_callback(GuiButtonType result, InputType type, void* context);

typedef struct {
    int transport_type;
    int transition;
    int service_provider;
    int station_group_id;
    int station_id;
    int location_gate;
    bool location_gate_available;
    int device;
    int door;
    int side;
    bool device_available;
    int route_number;
    bool route_number_available;
    int mission;
    bool mission_available;
    int vehicle_id;
    bool vehicle_id_available;
    int used_contract;
    bool used_contract_available;
    DateTime date;
} NavigoCardEvent;

typedef struct {
    int app_version;
    int country_num;
    int network_num;
    DateTime end_dt;
} NavigoCardEnv;

typedef struct {
    int card_status;
    int commercial_id;
} NavigoCardHolder;

typedef struct {
    int tariff;
    int serial_number;
    bool serial_number_available;
    int pay_method;
    bool pay_method_available;
    double price_amount;
    bool price_amount_available;
    DateTime start_date;
    DateTime end_date;
    bool end_date_available;
    int zones[5];
    bool zones_available;
    DateTime sale_date;
    int sale_agent;
    int sale_device;
    int status;
    int authenticator;
} NavigoCardContract;

typedef struct {
    NavigoCardEnv environment;
    NavigoCardHolder holder;
    NavigoCardContract* contracts;
    NavigoCardEvent* events;
    int ticket_count;
} NavigoCardData;

typedef struct {
    Metroflip* app;
    NavigoCardData* card;
    int page_id;
    // mutex
    FuriMutex* mutex;
} NavigoContext;

// Service Providers
static const char* SERVICE_PROVIDERS[] = {
    [2] = "SNCF",
    [3] = "RATP",
    [115] = "CSO (VEOLIA)",
    [116] = "R'Bus (VEOLIA)",
    [156] = "Phebus",
    [175] = "RATP (Veolia Transport Nanterre)"};

// Transport Types
static const char* TRANSPORT_LIST[] = {
    [1] = "Bus Urbain",
    [2] = "Bus Interurbain",
    [3] = "Metro",
    [4] = "Tram",
    [5] = "Train",
    [8] = "Parking"};

typedef enum {
    BUS_URBAIN = 1,
    BUS_INTERURBAIN = 2,
    METRO = 3,
    TRAM = 4,
    TRAIN = 5,
    PARKING = 8
} TRANSPORT_TYPE;

typedef enum {
    NAVIGO_EASY = 0,
    NAVIGO_DECOUVERTE = 1,
    NAVIGO_STANDARD = 2,
    NAVIGO_INTEGRAL = 6,
    IMAGINE_R = 14
} CARD_STATUS;

// Transition Types
static const char* TRANSITION_LIST[] = {
    [1] = "Validation en entree",
    [2] = "Validation en sortie",
    [4] = "Controle volant (a bord)",
    [5] = "Validation de test",
    [6] = "Validation en correspondance (entree)",
    [7] = "Validation en correspondance (sortie)",
    [9] = "Annulation de validation",
    [10] = "Validation en entree",
    [13] = "Distribution",
    [15] = "Invalidation"};

#endif // NAVIGO_H

static const char* METRO_STATION_LIST[32][16] =
    {[1] =
         {[0] = "Cite",
          [1] = "Saint-Michel",
          [4] = "Odeon",
          [5] = "Cluny - La Sorbonne",
          [6] = "Maubert - Mutualite",
          [7] = "Luxembourg",
          [8] = "Châtelet",
          [9] = "Les Halles",
          [10] = "Les Halles",
          [12] = "Louvre - Rivoli",
          [13] = "Pont Neuf",
          [14] = "Cite",
          [15] = "Hotel de Ville"},
     [2] =
         {[0] = "Rennes",
          [2] = "Cambronne",
          [3] = "Sevres - Lecourbe",
          [4] = "Segur",
          [6] = "Saint-François-Xavier",
          [7] = "Duroc",
          [8] = "Vaneau",
          [9] = "Sevres - Babylone",
          [10] = "Rue du Bac",
          [11] = "Rennes",
          [12] = "Saint-Sulpice",
          [14] = "Mabillon",
          [15] = "Saint-Germain-des-Pres"},
     [3] =
         {[0] = "Villette",
          [4] = "Porte de la Villette",
          [5] = "Aubervilliers - Pantin - Quatre Chemins",
          [6] = "Fort d'Aubervilliers",
          [7] = "La Courneuve - 8 Mai 1945",
          [9] = "Hoche",
          [10] = "Eglise de Pantin",
          [11] = "Bobigny - Pantin - Raymond Queneau",
          [12] = "Bobigny - Pablo Picasso"},
     [4] =
         {[0] = "Montparnasse",
          [2] = "Pernety",
          [3] = "Plaisance",
          [4] = "Gaite",
          [6] = "Edgar Quinet",
          [7] = "Vavin",
          [8] = "Montparnasse - Bienvenue",
          [12] = "Saint-Placide",
          [14] = "Notre-Dame-des-Champs"},
     [5] =
         {[0] = "Nation",
          [2] = "Robespierre",
          [3] = "Porte de Montreuil",
          [4] = "Maraichers",
          [5] = "Buzenval",
          [6] = "Rue des Boulets",
          [7] = "Porte de Vincennes",
          [9] = "Picpus",
          [10] = "Nation",
          [12] = "Avron",
          [13] = "Alexandre Dumas"},
     [6] =
         {[0] = "Saint-Lazare",
          [1] = "Malesherbes",
          [2] = "Monceau",
          [3] = "Villiers",
          [4] = "Quatre-Septembre",
          [5] = "Opera",
          [6] = "Auber",
          [7] = "Havre - Caumartin",
          [8] = "Saint-Lazare",
          [9] = "Saint-Lazare",
          [10] = "Saint-Augustin",
          [12] = "Europe",
          [13] = "Liege"},
     [7] =
         {[0] = "Auteuil",
          [3] = "Porte de Saint-Cloud",
          [7] = "Porte d'Auteuil",
          [8] = "eglise d'Auteuil",
          [9] = "Michel-Ange - Auteuil",
          [10] = "Michel-Ange - Molitor",
          [11] = "Chardon-Lagache",
          [12] = "Mirabeau",
          [14] = "Exelmans",
          [15] = "Jasmin"},
     [8] =
         {[0] = "Republique",
          [1] = "Rambuteau",
          [3] = "Arts et Metiers",
          [4] = "Jacques Bonsergent",
          [5] = "Goncourt",
          [6] = "Temple",
          [7] = "Republique",
          [10] = "Oberkampf",
          [11] = "Parmentier",
          [12] = "Filles du Calvaire",
          [13] = "Saint-Sebastien - Froissart",
          [14] = "Richard-Lenoir",
          [15] = "Saint-Ambroise"},
     [9] =
         {[0] = "Austerlitz",
          [1] = "Quai de la Gare",
          [2] = "Chevaleret",
          [4] = "Saint-Marcel",
          [7] = "Gare d'Austerlitz",
          [8] = "Gare de Lyon",
          [10] = "Quai de la Rapee"},
     [10] =
         {[0] = "Invalides",
          [1] = "Champs-elysees - Clemenceau",
          [2] = "Concorde",
          [3] = "Madeleine",
          [4] = "Bir-Hakeim",
          [7] = "ecole Militaire",
          [8] = "La Tour-Maubourg",
          [9] = "Invalides",
          [11] = "Saint-Denis - Universite",
          [12] = "Varenne",
          [13] = "Assemblee nationale",
          [14] = "Solferino"},
     [11] =
         {[0] = "Sentier",
          [1] = "Tuileries",
          [2] = "Palais Royal - Musee du Louvre",
          [3] = "Pyramides",
          [4] = "Bourse",
          [6] = "Grands Boulevards",
          [7] = "Richelieu - Drouot",
          [8] = "Bonne Nouvelle",
          [10] = "Strasbourg - Saint-Denis",
          [11] = "Château d'Eau",
          [13] = "Sentier",
          [14] = "Reaumur - Sebastopol",
          [15] = "etienne Marcel"},
     [12] =
         {[0] = "ile Saint-Louis",
          [1] = "Faidherbe - Chaligny",
          [2] = "Reuilly - Diderot",
          [3] = "Montgallet",
          [4] = "Censier - Daubenton",
          [5] = "Place Monge",
          [6] = "Cardinal Lemoine",
          [7] = "Jussieu",
          [8] = "Sully - Morland",
          [9] = "Pont Marie",
          [10] = "Saint-Paul",
          [12] = "Bastille",
          [13] = "Chemin Vert",
          [14] = "Breguet - Sabin",
          [15] = "Ledru-Rollin"},
     [13] =
         {[0] = "Daumesnil",
          [1] = "Porte Doree",
          [3] = "Porte de Charenton",
          [7] = "Bercy",
          [8] = "Dugommier",
          [10] = "Michel Bizot",
          [11] = "Daumesnil",
          [12] = "Bel-Air"},
     [14] =
         {[0] = "Italie",
          [2] = "Porte de Choisy",
          [3] = "Porte d'Italie",
          [4] = "Cite universitaire",
          [9] = "Maison Blanche",
          [10] = "Tolbiac",
          [11] = "Nationale",
          [12] = "Campo-Formio",
          [13] = "Les Gobelins",
          [14] = "Place d'Italie",
          [15] = "Corvisart"},
     [15] =
         {[0] = "Denfert",
          [1] = "Cour Saint-Emilion",
          [2] = "Porte d'Orleans",
          [3] = "Bibliotheque François Mitterrand",
          [4] = "Mouton-Duvernet",
          [5] = "Alesia",
          [6] = "Olympiades",
          [8] = "Glaciere",
          [9] = "Saint-Jacques",
          [10] = "Raspail",
          [14] = "Denfert-Rochereau"},
     [16] =
         {[0] = "Felix Faure",
          [1] = "Falguiere",
          [2] = "Pasteur",
          [3] = "Volontaires",
          [4] = "Vaugirard",
          [5] = "Convention",
          [6] = "Porte de Versailles",
          [9] = "Balard",
          [10] = "Lourmel",
          [11] = "Boucicaut",
          [12] = "Felix Faure",
          [13] = "Charles Michels",
          [14] = "Javel - Andre Citroen"},
     [17] =
         {[0] = "Passy",
          [2] = "Porte Dauphine",
          [4] = "La Motte-Picquet - Grenelle",
          [5] = "Commerce",
          [6] = "Avenue emile Zola",
          [7] = "Dupleix",
          [8] = "Passy",
          [9] = "Ranelagh",
          [11] = "La Muette",
          [13] = "Rue de la Pompe",
          [14] = "Boissiere",
          [15] = "Trocadero"},
     [18] =
         {[0] = "Etoile",
          [1] = "Iena",
          [3] = "Alma - Marceau",
          [4] = "Miromesnil",
          [5] = "Saint-Philippe du Roule",
          [7] = "Franklin D. Roosevelt",
          [8] = "George V",
          [9] = "Kleber",
          [10] = "Victor Hugo",
          [11] = "Argentine",
          [12] = "Charles de Gaulle - Itoile",
          [14] = "Ternes",
          [15] = "Courcelles"},
     [19] =
         {[0] = "Clichy - Saint Ouen",
          [1] = "Mairie de Clichy",
          [2] = "Gabriel Peri",
          [3] = "Les Agnettes",
          [4] = "Asnieres - Gennevilliers - Les Courtilles",
          [9] = "La Chapelle)",
          [10] = "Garibaldi",
          [11] = "Mairie de Saint-Ouen",
          [13] = "Carrefour Pleyel",
          [14] = "Saint-Denis - Porte de Paris",
          [15] = "Basilique de Saint-Denis"},
     [20] =
         {[0] = "Montmartre",
          [1] = "Porte de Clignancourt",
          [6] = "Porte de la Chapelle",
          [7] = "Marx Dormoy",
          [9] = "Marcadet - Poissonniers",
          [10] = "Simplon",
          [11] = "Jules Joffrin",
          [12] = "Lamarck - Caulaincourt"},
     [21] =
         {[0] = "Lafayette",
          [1] = "Chaussee d'Antin - La Fayette",
          [2] = "Le Peletier",
          [3] = "Cadet",
          [4] = "Château Rouge",
          [7] = "Barbes - Rochechouart",
          [8] = "Gare du Nord",
          [9] = "Gare de l'Est",
          [10] = "Poissonniere",
          [11] = "Château-Landon"},
     [22] =
         {[0] = "Buttes Chaumont",
          [1] = "Porte de Pantin",
          [2] = "Ourcq",
          [4] = "Corentin Cariou",
          [6] = "Crimee",
          [8] = "Riquet",
          [9] = "La Chapelle",
          [10] = "Louis Blanc",
          [11] = "Stalingrad",
          [12] = "Jaures",
          [13] = "Laumiere",
          [14] = "Bolivar",
          [15] = "Colonel Fabien"},
     [23] =
         {[0] = "Belleville",
          [2] = "Porte des Lilas",
          [3] = "Mairie des Lilas",
          [4] = "Porte de Bagnolet",
          [5] = "Gallieni",
          [8] = "Place des Fetes",
          [9] = "Botzaris",
          [10] = "Danube",
          [11] = "Pre Saint-Gervais",
          [13] = "Buttes Chaumont",
          [14] = "Jourdain",
          [15] = "Telegraphe"},
     [24] =
         {[0] = "Pere Lachaise",
          [1] = "Voltaire",
          [2] = "Charonne",
          [4] = "Pere Lachaise",
          [5] = "Menilmontant",
          [6] = "Rue Saint-Maur",
          [7] = "Philippe Auguste",
          [8] = "Saint-Fargeau",
          [9] = "Pelleport",
          [10] = "Gambetta",
          [12] = "Belleville",
          [13] = "Couronnes",
          [14] = "Pyrenees"},
     [25] =
         {[0] = "Charenton",
          [2] = "Croix de Chavaux",
          [3] = "Mairie de Montreuil",
          [4] = "Maisons-Alfort - Les Juilliottes",
          [5] = "Creteil - L'echat",
          [6] = "Creteil - Universite",
          [7] = "Creteil - Prefecture",
          [8] = "Saint-Mande",
          [10] = "Berault",
          [11] = "Château de Vincennes",
          [12] = "Liberte",
          [13] = "Charenton - ecoles",
          [14] = "ecole veterinaire de Maisons-Alfort",
          [15] = "Maisons-Alfort - Stade"},
     [26] =
         {[0] = "Ivry - Villejuif",
          [3] = "Porte d'Ivry",
          [4] = "Pierre et Marie Curie",
          [5] = "Mairie d'Ivry",
          [6] = "Le Kremlin-Bicetre",
          [7] = "Villejuif - Leo Lagrange",
          [8] = "Villejuif - Paul Vaillant-Couturier",
          [9] = "Villejuif - Louis Aragon"},
     [27] =
         {[0] = "Vanves",
          [2] = "Porte de Vanves",
          [7] = "Malakoff - Plateau de Vanves",
          [8] = "Malakoff - Rue etienne Dolet",
          [9] = "Châtillon - Montrouge"},
     [28] =
         {[0] = "Issy",
          [2] = "Corentin Celton",
          [3] = "Mairie d'Issy",
          [8] = "Marcel Sembat",
          [9] = "Billancourt",
          [10] = "Pont de Sevres"},
     [29] =
         {[0] = "Levallois",
          [4] = "Boulogne - Jean Jaures",
          [5] = "Boulogne - Pont de Saint-Cloud",
          [8] = "Les Sablons",
          [9] = "Pont de Neuilly",
          [10] = "Esplanade de la Defense",
          [11] = "La Defense",
          [12] = "Porte de Champerret",
          [13] = "Louise Michel",
          [14] = "Anatole France",
          [15] = "Pont de Levallois - Becon"},
     [30] =
         {[0] = "Pereire",
          [1] = "Porte Maillot",
          [4] = "Wagram",
          [5] = "Pereire",
          [8] = "Brochant",
          [9] = "Porte de Clichy",
          [12] = "Guy Moquet",
          [13] = "Porte de Saint-Ouen"},
     [31] = {
         [0] = "Pigalle",
         [2] = "Funiculaire de Montmartre (station inferieure)",
         [3] = "Funiculaire de Montmartre (station superieure)",
         [4] = "Anvers",
         [5] = "Abbesses",
         [6] = "Pigalle",
         [7] = "Blanche",
         [8] = "Trinite - d'Estienne d'Orves",
         [9] = "Notre-Dame-de-Lorette",
         [10] = "Saint-Georges",
         [12] = "Rome",
         [13] = "Place de Clichy",
         [14] = "La Fourche"}};

static const char* TRAIN_LINES_LIST[77] = {
    [1] = "RER B",         [3] = "RER B",         [6] = "RER A",         [14] = "RER B",
    [15] = "RER B",        [16] = "RER A",        [17] = "RER A",        [18] = "RER B",
    [20] = "Transilien P", [21] = "Transilien P", [22] = "T4",           [23] = "Transilien P",
    [26] = "RER A",        [28] = "RER B",        [30] = "Transilien L", [31] = "Transilien L",
    [32] = "Transilien J", [33] = "RER A",        [35] = "Transilien J", [40] = "RER D",
    [41] = "RER C",        [42] = "RER C",        [43] = "Transilien R", [44] = "Transilien R",
    [45] = "RER D",        [50] = "Transilien H", [51] = "Transilien K", [52] = "RER D",
    [53] = "Transilien H", [54] = "Transilien J", [55] = "RER C",        [56] = "Transilien H",
    [57] = "Transilien H", [60] = "Transilien N", [61] = "Transilien N", [63] = "RER C",
    [64] = "RER C",        [65] = "Transilien V", [70] = "RER B",        [72] = "Transilien J",
    [73] = "Transilien J", [75] = "RER C",        [76] = "RER C"};

static const char* TRAIN_STATION_LIST[77][19] = {
    [1] = {[0] = "Châtelet-Les Halles", [1] = "Châtelet-Les Halles", [7] = "Luxembourg"},
    [3] = {[0] = "Saint-Michel Notre-Dame"},
    [6] = {[0] = "Auber", [6] = "Auber"},
    [14] = {[4] = "Cite Universitaire"},
    [15] = {[12] = "Port Royal"},
    [16] =
        {[1] = "Nation",
         [2] = "Fontenay-sous-Bois | Vincennes",
         [3] = "Joinville-le-Pont | Nogent-sur-Marne",
         [4] = "Saint-Maur Creteil",
         [5] = "Le Parc de Saint-Maur",
         [6] = "Champigny",
         [7] = "La Varenne-Chennevieres",
         [8] = "Boissy-Saint-Leger | Sucy Bonneuil"},
    [17] =
        {[1] = "Charles de Gaulle-Etoile",
         [4] = "La Defense (Grande Arche)",
         [5] = "Nanterre-Ville",
         [6] = "Rueil-Malmaison",
         [8] = "Chatou-Croissy",
         [9] = "Le Vesinet-Centre | Le Vesinet-Le Pecq | Saint-Germain-en-Laye"},
    [18] =
        {[0] = "Denfert-Rochereau",
         [1] = "Gentilly",
         [2] = "Arcueil-Cachan | Laplace",
         [3] = "Bagneux | Bourg-la-Reine",
         [4] = "La Croix-de-Berny | Parc de Sceaux",
         [5] = "Antony | Fontaine-Michalon | Les Baconnets",
         [6] = "Massy-Palaiseau | Massy-Verrieres",
         [7] = "Palaiseau Villebon | Palaiseau",
         [8] = "Lozere",
         [9] = "Le Guichet | Orsay-Ville",
         [10] =
             "Bures-sur-Yvette | Courcelle-sur-Yvette | Gif-sur-Yvette | La Hacquiniere | Saint-Remy-les-Chevreuse"},
    [20] =
        {[1] = "Gare de l'Est",
         [4] = "Pantin",
         [5] = "Noisy-le-Sec",
         [6] = "Bondy",
         [7] = "Gagny | Le Raincy Villemomble Montfermeil",
         [9] = "Chelles Gournay | Le Chenay Gagny",
         [10] = "Vaires Torcy",
         [11] = "Lagny-Thorigny",
         [13] = "Esbly",
         [14] = "Meaux",
         [15] = "Changis-Saint-Jean | Isles-Armentieres Congis | Lizy-sur-Ourcq | Trilport",
         [16] = "Crouy-sur-Ourcq | La Ferte-sous-Jouarre | Nanteuil Saacy"},
    [21] =
        {[5] = "Rosny-Bois-Perrier | Rosny-sous-Bois | Val de Fontenay",
         [6] = "Nogent Le-Perreux",
         [7] = "Les Boullereaux Champigny",
         [8] = "Villiers-sur-Marne Plessis-Trevise",
         [9] = "Les Yvris Noisy-le-Grand",
         [10] = "Emerainville Pontault-Combault | Roissy-en-Brie",
         [11] = "Ozoir-la-Ferriere",
         [12] = "Gretz-Armainvilliers | Tournan",
         [15] =
             "Courquetaine | Faremoutiers Pommeuse | Guerard La-Celle-sur-Morin | Liverdy en Brie | Marles-en-Brie | Mormant | Mortcerf | Mouroux | Ozouer le voulgis | Verneuil-l'Etang | Villepatour - Presles | Yebles - Guignes | Yebles",
         [16] =
             "Chailly Boissy-le-Châtel | Chauffry | Coulommiers | Jouy-sur-Morin Le-Marais | Nangis | Saint-Remy-la-Vanne | Saint-Simeon",
         [17] =
             "Champbenoist-Poigny | La Ferte-Gaucher | Longueville | Provins | Sainte-Colombe-Septveilles",
         [18] = "Flamboin | Meilleray | Villiers St Georges"},
    [22] =
        {[7] =
             "Allee de la Tour-Rendez-Vous | La Remise-a-Jorelle | Les Coquetiers | Les Pavillons-sous-Bois",
         [8] = "Gargan",
         [9] = "Freinville Sevran | L'Abbaye"},
    [23] =
        {[13] = "Couilly Saint-Germain Quincy | Les Champs-Forts | Montry Conde",
         [14] = "Crecy-en-Brie La Chapelle | Villiers-Montbarbin"},
    [26] =
        {[5] = "Val de Fontenay",
         [6] = "Bry-sur-Marne | Neuilly-Plaisance",
         [7] = "Noisy-le-Grand (Mont d'Est)",
         [8] = "Noisy-Champs",
         [10] = "Lognes | Noisiel | Torcy",
         [11] = "Bussy-Saint-Georges",
         [12] = "Val d'europe",
         [13] = "Marne-la-Vallee Chessy"},
    [28] = {[4] = "Fontenay-aux-Roses | Robinson | Sceaux"},
    [30] =
        {[1] = "Gare Saint-Lazare",
         [3] = "Pont Cardinet",
         [4] =
             "Asnieres | Becon-les-Bruyeres | Clichy Levallois | Courbevoie | La Defense (Grande Arche)",
         [5] = "Puteaux | Suresnes Mont-Valerien",
         [7] = "Garches Marne-la-Coquette | Le Val d'Or | Saint-Cloud",
         [8] = "Vaucresson",
         [9] = "Bougival | La Celle-Saint-Cloud | Louveciennes | Marly-le-Roi",
         [10] = "L'Etang-la-Ville | Saint-Nom-la-Breteche Foret de Marly"},
    [31] =
        {[7] = "Chaville-Rive Droite | Sevres Ville-d'Avray | Viroflay-Rive Droite",
         [8] = "Montreuil | Versailles-Rive Droite"},
    [32] =
        {[5] = "La Garenne-Colombes | Les Vallees | Nanterre-Universite",
         [7] = "Houilles Carrieres-sur-Seine | Sartrouville",
         [9] = "Maisons-Laffitte",
         [10] = "Poissy",
         [11] = "Villennes-sur-Seine",
         [12] = "Les Clairieres de Verneuil | Vernouillet Verneuil",
         [13] = "Aubergenville-Elisabethville | Les Mureaux",
         [14] = "Epone Mezieres",
         [16] = "Bonnieres | Mantes-Station | Mantes-la-Jolie | Port-Villez | Rosny-sur-Seine"},
    [33] =
        {[10] = "Acheres-Grand-Cormier | Acheres-Ville",
         [11] = "Cergy-Prefecture | Neuville-Universite",
         [12] = "Cergy-Saint-Christophe | Cergy-le-Haut"},
    [35] =
        {[4] = "Bois-Colombes",
         [5] = "Colombes | Le Stade",
         [6] = "Argenteuil | Argenteuil",
         [8] = "Cormeilles-en-Parisis | Val d'Argenteuil | Val d'Argenteuil",
         [9] = "Herblay | La Frette Montigny",
         [10] = "Conflans-Fin d'Oise | Conflans-Sainte-Honorine",
         [11] = "Andresy | Chanteloup-les-Vignes | Maurecourt",
         [12] = "Triel-sur-Seine | Vaux-sur-Seine",
         [13] = "Meulan Hadricourt | Thun-le-Paradis",
         [14] = "Gargenville | Juziers",
         [15] = "Issou Porcheville | Limay",
         [16] = "Breval | Menerville"},
    [40] =
        {[1] = "Gare de Lyon",
         [5] = "Le Vert de Maisons | Maisons-Alfort Alfortville",
         [6] = "Villeneuve-Prairie",
         [7] = "Villeneuve-Triage",
         [8] = "Villeneuve-Saint-Georges",
         [9] = "Juvisy | Vigneux-sur-Seine",
         [10] = "Ris-Orangis | Viry-Châtillon",
         [11] = "Evry Val de Seine | Grand-Bourg",
         [12] = "Corbeil-Essonnes | Mennecy | Moulin-Galant",
         [13] = "Ballancourt | Fontenay le Vicomte",
         [14] = "La Ferte-Alais",
         [16] = "Boutigny | Maisse",
         [17] = "Boigneville | Buno-Gironville"},
    [41] =
        {[0] = "Musee d'Orsay | Saint-Michel Notre-Dame",
         [1] = "Gare d'Austerlitz",
         [2] = "Bibliotheque-Francois Mitterrand",
         [4] = "Ivry-sur-Seine | Vitry-sur-Seine",
         [5] = "Choisy-le-Roi | Les Ardoines",
         [7] = "Villeneuve-le-Roi",
         [8] = "Ablon",
         [9] = "Athis-Mons"},
    [42] =
        {[9] = "Epinay-sur-Orge | Savigny-sur-Orge",
         [10] = "Sainte-Genevieve-des-Bois",
         [11] = "Saint-Michel-sur-Orge",
         [12] = "Bretigny-sur-Orge | Marolles-en-Hurepoix",
         [13] = "Bouray | Lardy",
         [14] = "Chamarande | Etampes | Etrechy",
         [16] = "Saint-Martin d'Etampes",
         [17] = "Guillerval"},
    [43] =
        {[9] = "Montgeron Crosne | Yerres",
         [10] = "Brunoy",
         [11] = "Boussy-Saint-Antoine | Combs-la-Ville Quincy",
         [12] = "Lieusaint Moissy",
         [13] = "Cesson | Savigny-le-Temple Nandy",
         [15] = "Le Mee | Melun",
         [16] = "Chartrettes | Fontaine-le-Port | Livry-sur-Seine",
         [17] =
             "Champagne-sur-Seine | Hericy | La Grande Paroisse | Vernou-sur-Seine | Vulaines-sur-Seine Samoreau"},
    [44] =
        {[12] = "Essonnes-Robinson | Villabe",
         [13] = "Coudray-Montceaux | Le Plessis-Chenet-IBM | Saint-Fargeau",
         [14] = "Boissise-le-Roi | Ponthierry Pringy",
         [15] = "Vosves",
         [16] = "Bois-le-Roi",
         [17] =
             "Bagneaux-sur-Loing | Bourron-Marlotte Grez | Fontainebleau-Avon | Montereau | Montigny-sur-Loing | Moret Veneux-les-Sablons | Nemours Saint-Pierre | Saint-Mammes | Souppes | Thomery"},
    [45] =
        {[10] = "Grigny-Centre",
         [11] = "Evry Courcouronnes | Orangis Bois de l'Epine",
         [12] = "Le Bras-de-Fer - Evry Genopole"},
    [50] =
        {[0] = "Haussmann-Saint-Lazare",
         [1] = "Gare du Nord | Magenta | Paris-Nord",
         [5] = "Epinay-Villetaneuse | Saint-Denis | Sevres-Rive Gauche",
         [6] = "La Barre-Ormesson",
         [7] = "Champ de Courses d'Enghien | Enghien-les-Bains",
         [8] = "Ermont-Eaubonne | Ermont-Halte | Gros-Noyer Saint-Prix",
         [9] = "Saint-Leu-La-Foret | Taverny | Vaucelles",
         [10] = "Bessancourt | Frepillon | Mery",
         [11] = "Meriel | Valmondois",
         [12] = "Bruyeres-sur-Oise | Champagne-sur-Oise | L'Isle-Adam Parmain | Persan Beaumont"},
    [51] =
        {[4] = "La Courneuve-Aubervilliers | La Plaine-Stade de France",
         [5] = "Le Bourget",
         [7] = "Blanc-Mesnil | Drancy",
         [8] = "Aulnay-sous-Bois",
         [9] = "Sevran Livry | Vert-Galant",
         [10] = "Villeparisis",
         [11] = "Compans | Mitry-Claye",
         [12] = "Dammartin Juilly Saint-Mard | Thieux Nantouillet"},
    [52] =
        {[5] = "Stade de France-Saint-Denis",
         [6] = "Pierrefitte Stains",
         [7] = "Garges-Sarcelles",
         [8] = "Villiers-le-Bel (Gonesse - Arnouville)",
         [10] = "Goussainville | Les Noues | Louvres",
         [11] = "La Borne-Blanche | Survilliers-Fosses"},
    [53] =
        {[6] = "Deuil Montmagny",
         [7] = "Groslay",
         [8] = "Sarcelles Saint-Brice",
         [9] = "Domont | Ecouen Ezanville",
         [10] = "Bouffemont Moisselles | Montsoult Maffliers",
         [11] = "Belloy-Saint-Martin | Luzarches | Seugy | Viarmes | Villaines"},
    [54] =
        {[8] = "Cernay",
         [9] = "Franconville Plessis-Bouchard | Montigny-Beauchamp",
         [10] = "Pierrelaye",
         [11] = "Pontoise | Saint-Ouen-l'Aumone-Liesse",
         [12] = "Boissy-l'Aillerie | Osny",
         [15] = "Chars | Montgeroult Courcelles | Santeuil Le Perchay | Us"},
    [55] =
        {[0] =
             "Avenue Foch | Avenue Henri-Martin | Boulainvilliers | Kennedy Radio-France | Neuilly-Porte Maillot (Palais des congres)",
         [1] = "Pereire-Levallois",
         [2] = "Porte de Clichy",
         [3] = "Saint-Ouen",
         [4] = "Les Gresillons",
         [5] = "Gennevilliers",
         [6] = "Epinay-sur-Seine",
         [7] = "Saint-Gratien"},
    [56] = {[11] = "Auvers-sur-Oise | Chaponval | Epluches | Pont Petit"},
    [57] = {[11] = "Presles Courcelles", [12] = "Nointel Mours"},
    [60] =
        {[1] = "Gare Montparnasse",
         [4] = "Clamart | Vanves Malakoff",
         [5] = "Bellevue | Bievres | Meudon",
         [6] = "Chaville-Rive Gauche | Chaville-Velizy | Viroflay-Rive Gauche",
         [7] = "Versailles-Chantiers",
         [10] = "Saint-Cyr",
         [11] = "Saint-Quentin-en-Yvelines - Montigny le Bretonneux | Trappes",
         [12] = "Coignieres | La Verriere",
         [13] = "Les Essarts-le-Roi",
         [14] = "Le Perray | Rambouillet",
         [15] = "Gazeran"},
    [61] =
        {[10] = "Fontenay-le-Fleury",
         [11] = "Villepreux Les-Clayes",
         [12] = "Plaisir Grignon | Plaisir Les-Clayes",
         [13] = "Beynes | Mareil-sur-Mauldre | Maule | Nezel Aulnay",
         [15] =
             "Garancieres La-Queue | Montfort-l'Amaury Mere | Orgerus Behoust | Tacoigneres Richebourg | Villiers Neauphle Pontchartrain",
         [16] = "Houdan"},
    [63] = {[7] = "Porchefontaine | Versailles-Rive Gauche"},
    [64] =
        {[0] = "Invalides | Pont de l'alma",
         [1] = "Champ de Mars-Tour Eiffel",
         [2] = "Javel",
         [3] = "Boulevard Victor - Pont du Garigliano | Issy-Val de Seine | Issy",
         [5] = "Meudon-Val-Fleury"},
    [65] =
        {[8] = "Jouy-en-Josas | Petit-Jouy-les-Loges",
         [9] = "Vauboyen",
         [10] = "Igny",
         [11] = "Massy-Palaiseau",
         [12] = "Longjumeau",
         [13] = "Chilly-Mazarin",
         [14] = "Gravigny-Balizy | Petit-Vaux"},
    [70] =
        {[9] = "Parc des Expositions | Sevran-Beaudottes | Villepinte",
         [10] = "Aeroport Charles de Gaulle"},
    [72] = {[7] = "Sannois"},
    [73] = {[11] = "Eragny Neuville | Saint-Ouen-l'Aumone (Eglise)"},
    [75] =
        {[7] = "Les Saules | Orly-Ville",
         [9] = "Pont de Rungis Aeroport d'Orly | Rungis-La Fraternelle",
         [10] = "Chemin d'Antony",
         [12] = "Massy-Verrieres | Arpajon"},
    [76] =
        {[12] = "Egly | La Norville Saint-Germain-les-Arpajon",
         [13] = "Breuillet Bruyeres-le-Châtel | Breuillet-Village | Saint-Cheron",
         [14] = "Sermaise",
         [15] = "Dourdan | Dourdan-la-Foret"},
};

#endif // METRO_LIST_H
