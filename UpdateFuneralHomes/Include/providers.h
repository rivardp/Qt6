// providers.h

#ifndef PROVIDERS_H
#define PROVIDERS_H

#include <string>
#include "tchar.h"

enum PROVIDER {Undefined, Legacy , Passages, LifeMoments, AFHA, Brandon, CooperativeFuneraire, Phaonix, DomainFuneraire, SalonsFuneraire,
               AvisDeDeces, GreatWest, ObitTree, EchoVita, SaltWire, BlackPress, Aberdeen,
               Batesville = 1000, SIDS, BrowseAll, DignityMemorial, FosterMcGarvey, CFS, Frazer, Alternatives, FuneralTech, WordPress,
               FrontRunner, FuneralOne, WebStorm, GasLamp, ClickTributes, ConnellyMcKinley, Arbor, SiteWyze, ThinkingThru, Codesign,
               Shape5, TributeArchive, YAS, WFFH, FHW, Crew, Jelly, Nirvana, Bergetti, PacificByte,
               ROI, Vernon, Gustin, Ashlean, Halifax, Specialty, ReneBabin, Smiths, Seabreeze, RedSands,
               AdGraphics, Websites, MPG, MediaHosts, DragonFly, MyFavourite, Coop, EverTech, MacLean, MCG,
               UNI, WebSolutions, Citrus, M5i, TNours, Signature, SandFire, Carve, SRS, BrandAgent,
               EPmedia, Linkhouse, CreativeMedia, LinkWeb, JoshPascoe, ChadSimpson, MarketingImages, ESite, SquareSpace, Eggs,
               MFH, ONeil, LeadDog, Back2Front, Cats, InView, CreativeOne, RKD, SDP, Globalia,
               Vortex, Elegant, YellowPages, Shooga, NBL, WPBakery, Imago, TroisJoueur, Ubeo, Acolyte,
               Morin, Taiga, Zonart, PubliWeb, DirectImpact, SoleWeb, Voyou, Scrypta, Jaimonsite, Saguenay,
               Lithium, Cameleon, LogiAction, BLsolutions, ToraPro, Axanti, ADN, B367, Tomahawk, LeSaint,
               Caza, Tegara, NMedia, Webs, Descary, Tonik, Kaleidos, Gemini, Alias, Cible,
               Web2u, District4Web, Cake, J27, NetRevolution, ImageXpert, Reactif, Boite, Orage, Kerozen,
               InoVision, FRM, PassageCoop, Ah, JBCote, BlackCreek, CityMax, SYGIF, PortNeuf, Burke,
               Canadian, BallaMedia, Jac, Ministry, Multinet, PropulC, Nexion, LCProduction, Absolu, SuiteB,
               Map, iClic, Bouille, Pub30, Theories14, Techlogical, GyOrgy, GemWebb, RedChair, ExtremeSurf,
               Cahoots, Tride, Jensii, InterWeb, Brown, Tukio, WebCemeteries, Etincelle,
               MikePurdy = 2000, BowRiver, Serenity, McInnis, Sturgeon, CornerStone, Pierson, Trinity, CelebrateLife, Funks,
               WowFactor, Dalmeny, Hansons, IDOmedia, Martens, Shine, Simply, McCall, Care, Ancient,
               Amherst, Bowers, Heritage, Koru, Kowalchuk, Loehmer, Doyle, Ethical, Integrity, Direct,
               SMC, Belvedere, Davidson, Carnell, Dunphy, JOsmond, TivaHost, KMF, AMG, Orillia,
               OSM, Alcock, Abstract, Beechwood, Benjamins, Berthiaume, Blenheim, Brenneman, Cardinal, Carson,
               Turner, Cole, Eagleson, FirstMemorial, Haine, PineCrest, RHB, Rhody, Simpler, Steadman,
               Steeles, Bridge, McCormack, Brunet, TurnerPorter, TurnerFamily, VanHeck, TBK, Whelan, Aeterna,
               Actuel, Dupuis, HGDivision, Jacques, Joliette, Rajotte, BM, Jodoin, Fournier, Desnoyers,
               Desrosiers, MontPetit, Parent, RichardPhilibert, Kane, Gaudet, LaurentNormand, NouvelleVie, Santerre, Shields,
               Gamache, Landry, StLouis, McGerrigle, Paperman, Poissant, Legare, Longpre, Lanaudiere, Theriault,
               Voluntas, Wilbrod, Hodges, Bergeron, Passage, Granit, Affordable, LFC, LifeTransitions, Davis,
               MacLark, Fallis, Timiskaming, Garrett, Smith, Picard, Richelieu, Roy, CharleVoix, Aurora,
               Montcalm, Trahan, Laurent, Eternel, Ruel, Hamel, CremAlt, London, Dryden, Forest,
               JasonSmith, Lampman, ecoPassages, Peaceful, Ranger, People, Whitcroft, TriCity, LegacyCardstrom, Wiebe,
               Arimathea, GFournier, CharleVoix2, Harmonia, Omega, HeritageWP, Ouellet, HommageNB, Drake, CityLine,
               Komitas, Driftwood};

// Search parameters

static const std::wstring LegacyProvinces[] = { _T("Alberta"), _T("British-Columbia"), _T("Manitoba"), _T("New-Brunswick"), _T("Newfoundland"),
                                                _T("Nova-Scotia"), _T("Northwest-Territories"), _T("Ontario"), _T("Quebec"), _T("Saskatchewan") };


#endif
