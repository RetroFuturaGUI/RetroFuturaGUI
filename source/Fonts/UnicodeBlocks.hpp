/* First/last codepoint bounds for each Unicode block, used to request glyph
    ranges from FontManager without hardcoding magic numbers at call sites.
    Made by tools/make_unicode_blocks.awk from Unicode 18.0's Blocks.txt: for a new Unicode version, run it again
    rather than editing this by hand. The bounds are the whole block, not only its assigned characters, so characters
    added to a block later are covered too. Basic Latin is split in two: its control characters are ControlBlock, which
    stays out of UnicodeBlocks because FontManager::blockKeyOf returns 0 for "no block".
*/
#pragma once
#include "IncludeHelper.hpp"
#include <array>

#define ControlBlockFirst 0x0U
#define ControlBlockLast 0x1FU
#define BasicLatinFirst 0x20U
#define BasicLatinLast 0x7FU
#define Latin1SupplementFirst 0x80U
#define Latin1SupplementLast 0xFFU
#define LatinExtendedAFirst 0x100U
#define LatinExtendedALast 0x17FU
#define LatinExtendedBFirst 0x180U
#define LatinExtendedBLast 0x24FU
#define IPAExtensionsFirst 0x250U
#define IPAExtensionsLast 0x2AFU
#define SpacingModifierLettersFirst 0x2B0U
#define SpacingModifierLettersLast 0x2FFU
#define CombiningDiacriticalMarksFirst 0x300U
#define CombiningDiacriticalMarksLast 0x36FU
#define GreekandCopticFirst 0x370U
#define GreekandCopticLast 0x3FFU
#define CyrillicFirst 0x400U
#define CyrillicLast 0x4FFU
#define CyrillicSupplementFirst 0x500U
#define CyrillicSupplementLast 0x52FU
#define ArmenianFirst 0x530U
#define ArmenianLast 0x58FU
#define HebrewFirst 0x590U
#define HebrewLast 0x5FFU
#define ArabicFirst 0x600U
#define ArabicLast 0x6FFU
#define SyriacFirst 0x700U
#define SyriacLast 0x74FU
#define ArabicSupplementFirst 0x750U
#define ArabicSupplementLast 0x77FU
#define ThaanaFirst 0x780U
#define ThaanaLast 0x7BFU
#define NKoFirst 0x7C0U
#define NKoLast 0x7FFU
#define SamaritanFirst 0x800U
#define SamaritanLast 0x83FU
#define MandaicFirst 0x840U
#define MandaicLast 0x85FU
#define SyriacSupplementFirst 0x860U
#define SyriacSupplementLast 0x86FU
#define ArabicExtendedBFirst 0x870U
#define ArabicExtendedBLast 0x89FU
#define ArabicExtendedAFirst 0x8A0U
#define ArabicExtendedALast 0x8FFU
#define DevanagariFirst 0x900U
#define DevanagariLast 0x97FU
#define BengaliFirst 0x980U
#define BengaliLast 0x9FFU
#define GurmukhiFirst 0xA00U
#define GurmukhiLast 0xA7FU
#define GujaratiFirst 0xA80U
#define GujaratiLast 0xAFFU
#define OriyaFirst 0xB00U
#define OriyaLast 0xB7FU
#define TamilFirst 0xB80U
#define TamilLast 0xBFFU
#define TeluguFirst 0xC00U
#define TeluguLast 0xC7FU
#define KannadaFirst 0xC80U
#define KannadaLast 0xCFFU
#define MalayalamFirst 0xD00U
#define MalayalamLast 0xD7FU
#define SinhalaFirst 0xD80U
#define SinhalaLast 0xDFFU
#define ThaiFirst 0xE00U
#define ThaiLast 0xE7FU
#define LaoFirst 0xE80U
#define LaoLast 0xEFFU
#define TibetanFirst 0xF00U
#define TibetanLast 0xFFFU
#define MyanmarFirst 0x1000U
#define MyanmarLast 0x109FU
#define GeorgianFirst 0x10A0U
#define GeorgianLast 0x10FFU
#define HangulJamoFirst 0x1100U
#define HangulJamoLast 0x11FFU
#define EthiopicFirst 0x1200U
#define EthiopicLast 0x137FU
#define EthiopicSupplementFirst 0x1380U
#define EthiopicSupplementLast 0x139FU
#define CherokeeFirst 0x13A0U
#define CherokeeLast 0x13FFU
#define UnifiedCanadianAboriginalSyllabicsFirst 0x1400U
#define UnifiedCanadianAboriginalSyllabicsLast 0x167FU
#define OghamFirst 0x1680U
#define OghamLast 0x169FU
#define RunicFirst 0x16A0U
#define RunicLast 0x16FFU
#define TagalogFirst 0x1700U
#define TagalogLast 0x171FU
#define HanunooFirst 0x1720U
#define HanunooLast 0x173FU
#define BuhidFirst 0x1740U
#define BuhidLast 0x175FU
#define TagbanwaFirst 0x1760U
#define TagbanwaLast 0x177FU
#define KhmerFirst 0x1780U
#define KhmerLast 0x17FFU
#define MongolianFirst 0x1800U
#define MongolianLast 0x18AFU
#define UnifiedCanadianAboriginalSyllabicsExtendedFirst 0x18B0U
#define UnifiedCanadianAboriginalSyllabicsExtendedLast 0x18FFU
#define LimbuFirst 0x1900U
#define LimbuLast 0x194FU
#define TaiLeFirst 0x1950U
#define TaiLeLast 0x197FU
#define NewTaiLueFirst 0x1980U
#define NewTaiLueLast 0x19DFU
#define KhmerSymbolsFirst 0x19E0U
#define KhmerSymbolsLast 0x19FFU
#define BugineseFirst 0x1A00U
#define BugineseLast 0x1A1FU
#define TaiThamFirst 0x1A20U
#define TaiThamLast 0x1AAFU
#define CombiningDiacriticalMarksExtendedFirst 0x1AB0U
#define CombiningDiacriticalMarksExtendedLast 0x1AFFU
#define BalineseFirst 0x1B00U
#define BalineseLast 0x1B7FU
#define SundaneseFirst 0x1B80U
#define SundaneseLast 0x1BBFU
#define BatakFirst 0x1BC0U
#define BatakLast 0x1BFFU
#define LepchaFirst 0x1C00U
#define LepchaLast 0x1C4FU
#define OlChikiFirst 0x1C50U
#define OlChikiLast 0x1C7FU
#define CyrillicExtendedCFirst 0x1C80U
#define CyrillicExtendedCLast 0x1C8FU
#define GeorgianExtendedFirst 0x1C90U
#define GeorgianExtendedLast 0x1CBFU
#define SundaneseSupplementFirst 0x1CC0U
#define SundaneseSupplementLast 0x1CCFU
#define VedicExtensionsFirst 0x1CD0U
#define VedicExtensionsLast 0x1CFFU
#define PhoneticExtensionsFirst 0x1D00U
#define PhoneticExtensionsLast 0x1D7FU
#define PhoneticExtensionsSupplementFirst 0x1D80U
#define PhoneticExtensionsSupplementLast 0x1DBFU
#define CombiningDiacriticalMarksSupplementFirst 0x1DC0U
#define CombiningDiacriticalMarksSupplementLast 0x1DFFU
#define LatinExtendedAdditionalFirst 0x1E00U
#define LatinExtendedAdditionalLast 0x1EFFU
#define GreekExtendedFirst 0x1F00U
#define GreekExtendedLast 0x1FFFU
#define GeneralPunctuationFirst 0x2000U
#define GeneralPunctuationLast 0x206FU
#define SuperscriptsandSubscriptsFirst 0x2070U
#define SuperscriptsandSubscriptsLast 0x209FU
#define CurrencySymbolsFirst 0x20A0U
#define CurrencySymbolsLast 0x20CFU
#define CombiningDiacriticalMarksforSymbolsFirst 0x20D0U
#define CombiningDiacriticalMarksforSymbolsLast 0x20FFU
#define LetterlikeSymbolsFirst 0x2100U
#define LetterlikeSymbolsLast 0x214FU
#define NumberFormsFirst 0x2150U
#define NumberFormsLast 0x218FU
#define ArrowsFirst 0x2190U
#define ArrowsLast 0x21FFU
#define MathematicalOperatorsFirst 0x2200U
#define MathematicalOperatorsLast 0x22FFU
#define MiscellaneousTechnicalFirst 0x2300U
#define MiscellaneousTechnicalLast 0x23FFU
#define ControlPicturesFirst 0x2400U
#define ControlPicturesLast 0x243FU
#define OpticalCharacterRecognitionFirst 0x2440U
#define OpticalCharacterRecognitionLast 0x245FU
#define EnclosedAlphanumericsFirst 0x2460U
#define EnclosedAlphanumericsLast 0x24FFU
#define BoxDrawingFirst 0x2500U
#define BoxDrawingLast 0x257FU
#define BlockElementsFirst 0x2580U
#define BlockElementsLast 0x259FU
#define GeometricShapesFirst 0x25A0U
#define GeometricShapesLast 0x25FFU
#define MiscellaneousSymbolsFirst 0x2600U
#define MiscellaneousSymbolsLast 0x26FFU
#define DingbatsFirst 0x2700U
#define DingbatsLast 0x27BFU
#define MiscellaneousMathematicalSymbolsAFirst 0x27C0U
#define MiscellaneousMathematicalSymbolsALast 0x27EFU
#define SupplementalArrowsAFirst 0x27F0U
#define SupplementalArrowsALast 0x27FFU
#define BraillePatternsFirst 0x2800U
#define BraillePatternsLast 0x28FFU
#define SupplementalArrowsBFirst 0x2900U
#define SupplementalArrowsBLast 0x297FU
#define MiscellaneousMathematicalSymbolsBFirst 0x2980U
#define MiscellaneousMathematicalSymbolsBLast 0x29FFU
#define SupplementalMathematicalOperatorsFirst 0x2A00U
#define SupplementalMathematicalOperatorsLast 0x2AFFU
#define MiscellaneousSymbolsandArrowsFirst 0x2B00U
#define MiscellaneousSymbolsandArrowsLast 0x2BFFU
#define GlagoliticFirst 0x2C00U
#define GlagoliticLast 0x2C5FU
#define LatinExtendedCFirst 0x2C60U
#define LatinExtendedCLast 0x2C7FU
#define CopticFirst 0x2C80U
#define CopticLast 0x2CFFU
#define GeorgianSupplementFirst 0x2D00U
#define GeorgianSupplementLast 0x2D2FU
#define TifinaghFirst 0x2D30U
#define TifinaghLast 0x2D7FU
#define EthiopicExtendedFirst 0x2D80U
#define EthiopicExtendedLast 0x2DDFU
#define CyrillicExtendedAFirst 0x2DE0U
#define CyrillicExtendedALast 0x2DFFU
#define SupplementalPunctuationFirst 0x2E00U
#define SupplementalPunctuationLast 0x2E7FU
#define CJKRadicalsSupplementFirst 0x2E80U
#define CJKRadicalsSupplementLast 0x2EFFU
#define KangxiRadicalsFirst 0x2F00U
#define KangxiRadicalsLast 0x2FDFU
#define IdeographicDescriptionCharactersFirst 0x2FF0U
#define IdeographicDescriptionCharactersLast 0x2FFFU
#define CJKSymbolsandPunctuationFirst 0x3000U
#define CJKSymbolsandPunctuationLast 0x303FU
#define HiraganaFirst 0x3040U
#define HiraganaLast 0x309FU
#define KatakanaFirst 0x30A0U
#define KatakanaLast 0x30FFU
#define BopomofoFirst 0x3100U
#define BopomofoLast 0x312FU
#define HangulCompatibilityJamoFirst 0x3130U
#define HangulCompatibilityJamoLast 0x318FU
#define KanbunFirst 0x3190U
#define KanbunLast 0x319FU
#define BopomofoExtendedFirst 0x31A0U
#define BopomofoExtendedLast 0x31BFU
#define CJKStrokesFirst 0x31C0U
#define CJKStrokesLast 0x31EFU
#define KatakanaPhoneticExtensionsFirst 0x31F0U
#define KatakanaPhoneticExtensionsLast 0x31FFU
#define EnclosedCJKLettersandMonthsFirst 0x3200U
#define EnclosedCJKLettersandMonthsLast 0x32FFU
#define CJKCompatibilityFirst 0x3300U
#define CJKCompatibilityLast 0x33FFU
#define CJKUnifiedIdeographsExtensionAFirst 0x3400U
#define CJKUnifiedIdeographsExtensionALast 0x4DBFU
#define YijingHexagramSymbolsFirst 0x4DC0U
#define YijingHexagramSymbolsLast 0x4DFFU
#define CJKUnifiedIdeographsFirst 0x4E00U
#define CJKUnifiedIdeographsLast 0x9FFFU
#define YiSyllablesFirst 0xA000U
#define YiSyllablesLast 0xA48FU
#define YiRadicalsFirst 0xA490U
#define YiRadicalsLast 0xA4CFU
#define LisuFirst 0xA4D0U
#define LisuLast 0xA4FFU
#define VaiFirst 0xA500U
#define VaiLast 0xA63FU
#define CyrillicExtendedBFirst 0xA640U
#define CyrillicExtendedBLast 0xA69FU
#define BamumFirst 0xA6A0U
#define BamumLast 0xA6FFU
#define ModifierToneLettersFirst 0xA700U
#define ModifierToneLettersLast 0xA71FU
#define LatinExtendedDFirst 0xA720U
#define LatinExtendedDLast 0xA7FFU
#define SylotiNagriFirst 0xA800U
#define SylotiNagriLast 0xA82FU
#define CommonIndicNumberFormsFirst 0xA830U
#define CommonIndicNumberFormsLast 0xA83FU
#define PhagspaFirst 0xA840U
#define PhagspaLast 0xA87FU
#define SaurashtraFirst 0xA880U
#define SaurashtraLast 0xA8DFU
#define DevanagariExtendedFirst 0xA8E0U
#define DevanagariExtendedLast 0xA8FFU
#define KayahLiFirst 0xA900U
#define KayahLiLast 0xA92FU
#define RejangFirst 0xA930U
#define RejangLast 0xA95FU
#define HangulJamoExtendedAFirst 0xA960U
#define HangulJamoExtendedALast 0xA97FU
#define JavaneseFirst 0xA980U
#define JavaneseLast 0xA9DFU
#define MyanmarExtendedBFirst 0xA9E0U
#define MyanmarExtendedBLast 0xA9FFU
#define ChamFirst 0xAA00U
#define ChamLast 0xAA5FU
#define MyanmarExtendedAFirst 0xAA60U
#define MyanmarExtendedALast 0xAA7FU
#define TaiVietFirst 0xAA80U
#define TaiVietLast 0xAADFU
#define MeeteiMayekExtensionsFirst 0xAAE0U
#define MeeteiMayekExtensionsLast 0xAAFFU
#define EthiopicExtendedAFirst 0xAB00U
#define EthiopicExtendedALast 0xAB2FU
#define LatinExtendedEFirst 0xAB30U
#define LatinExtendedELast 0xAB6FU
#define CherokeeSupplementFirst 0xAB70U
#define CherokeeSupplementLast 0xABBFU
#define MeeteiMayekFirst 0xABC0U
#define MeeteiMayekLast 0xABFFU
#define HangulSyllablesFirst 0xAC00U
#define HangulSyllablesLast 0xD7AFU
#define HangulJamoExtendedBFirst 0xD7B0U
#define HangulJamoExtendedBLast 0xD7FFU
#define HighSurrogatesFirst 0xD800U
#define HighSurrogatesLast 0xDB7FU
#define HighPrivateUseSurrogatesFirst 0xDB80U
#define HighPrivateUseSurrogatesLast 0xDBFFU
#define LowSurrogatesFirst 0xDC00U
#define LowSurrogatesLast 0xDFFFU
#define PrivateUseAreaFirst 0xE000U
#define PrivateUseAreaLast 0xF8FFU
#define CJKCompatibilityIdeographsFirst 0xF900U
#define CJKCompatibilityIdeographsLast 0xFAFFU
#define AlphabeticPresentationFormsFirst 0xFB00U
#define AlphabeticPresentationFormsLast 0xFB4FU
#define ArabicPresentationFormsAFirst 0xFB50U
#define ArabicPresentationFormsALast 0xFDFFU
#define VariationSelectorsFirst 0xFE00U
#define VariationSelectorsLast 0xFE0FU
#define VerticalFormsFirst 0xFE10U
#define VerticalFormsLast 0xFE1FU
#define CombiningHalfMarksFirst 0xFE20U
#define CombiningHalfMarksLast 0xFE2FU
#define CJKCompatibilityFormsFirst 0xFE30U
#define CJKCompatibilityFormsLast 0xFE4FU
#define SmallFormVariantsFirst 0xFE50U
#define SmallFormVariantsLast 0xFE6FU
#define ArabicPresentationFormsBFirst 0xFE70U
#define ArabicPresentationFormsBLast 0xFEFFU
#define HalfwidthandFullwidthFormsFirst 0xFF00U
#define HalfwidthandFullwidthFormsLast 0xFFEFU
#define SpecialsFirst 0xFFF0U
#define SpecialsLast 0xFFFFU
#define LinearBSyllabaryFirst 0x10000U
#define LinearBSyllabaryLast 0x1007FU
#define LinearBIdeogramsFirst 0x10080U
#define LinearBIdeogramsLast 0x100FFU
#define AegeanNumbersFirst 0x10100U
#define AegeanNumbersLast 0x1013FU
#define AncientGreekNumbersFirst 0x10140U
#define AncientGreekNumbersLast 0x1018FU
#define AncientSymbolsFirst 0x10190U
#define AncientSymbolsLast 0x101CFU
#define PhaistosDiscFirst 0x101D0U
#define PhaistosDiscLast 0x101FFU
#define LycianFirst 0x10280U
#define LycianLast 0x1029FU
#define CarianFirst 0x102A0U
#define CarianLast 0x102DFU
#define CopticEpactNumbersFirst 0x102E0U
#define CopticEpactNumbersLast 0x102FFU
#define OldItalicFirst 0x10300U
#define OldItalicLast 0x1032FU
#define GothicFirst 0x10330U
#define GothicLast 0x1034FU
#define OldPermicFirst 0x10350U
#define OldPermicLast 0x1037FU
#define UgariticFirst 0x10380U
#define UgariticLast 0x1039FU
#define OldPersianFirst 0x103A0U
#define OldPersianLast 0x103DFU
#define DeseretFirst 0x10400U
#define DeseretLast 0x1044FU
#define ShavianFirst 0x10450U
#define ShavianLast 0x1047FU
#define OsmanyaFirst 0x10480U
#define OsmanyaLast 0x104AFU
#define OsageFirst 0x104B0U
#define OsageLast 0x104FFU
#define ElbasanFirst 0x10500U
#define ElbasanLast 0x1052FU
#define CaucasianAlbanianFirst 0x10530U
#define CaucasianAlbanianLast 0x1056FU
#define VithkuqiFirst 0x10570U
#define VithkuqiLast 0x105BFU
#define TodhriFirst 0x105C0U
#define TodhriLast 0x105FFU
#define LinearAFirst 0x10600U
#define LinearALast 0x1077FU
#define LatinExtendedFFirst 0x10780U
#define LatinExtendedFLast 0x107BFU
#define CypriotSyllabaryFirst 0x10800U
#define CypriotSyllabaryLast 0x1083FU
#define ImperialAramaicFirst 0x10840U
#define ImperialAramaicLast 0x1085FU
#define PalmyreneFirst 0x10860U
#define PalmyreneLast 0x1087FU
#define NabataeanFirst 0x10880U
#define NabataeanLast 0x108AFU
#define HatranFirst 0x108E0U
#define HatranLast 0x108FFU
#define PhoenicianFirst 0x10900U
#define PhoenicianLast 0x1091FU
#define LydianFirst 0x10920U
#define LydianLast 0x1093FU
#define SideticFirst 0x10940U
#define SideticLast 0x1095FU
#define MeroiticHieroglyphsFirst 0x10980U
#define MeroiticHieroglyphsLast 0x1099FU
#define MeroiticCursiveFirst 0x109A0U
#define MeroiticCursiveLast 0x109FFU
#define KharoshthiFirst 0x10A00U
#define KharoshthiLast 0x10A5FU
#define OldSouthArabianFirst 0x10A60U
#define OldSouthArabianLast 0x10A7FU
#define OldNorthArabianFirst 0x10A80U
#define OldNorthArabianLast 0x10A9FU
#define ManichaeanFirst 0x10AC0U
#define ManichaeanLast 0x10AFFU
#define AvestanFirst 0x10B00U
#define AvestanLast 0x10B3FU
#define InscriptionalParthianFirst 0x10B40U
#define InscriptionalParthianLast 0x10B5FU
#define InscriptionalPahlaviFirst 0x10B60U
#define InscriptionalPahlaviLast 0x10B7FU
#define PsalterPahlaviFirst 0x10B80U
#define PsalterPahlaviLast 0x10BAFU
#define OldTurkicFirst 0x10C00U
#define OldTurkicLast 0x10C4FU
#define OldHungarianFirst 0x10C80U
#define OldHungarianLast 0x10CFFU
#define HanifiRohingyaFirst 0x10D00U
#define HanifiRohingyaLast 0x10D3FU
#define GarayFirst 0x10D40U
#define GarayLast 0x10D8FU
#define RumiNumeralSymbolsFirst 0x10E60U
#define RumiNumeralSymbolsLast 0x10E7FU
#define YezidiFirst 0x10E80U
#define YezidiLast 0x10EBFU
#define ArabicExtendedCFirst 0x10EC0U
#define ArabicExtendedCLast 0x10EFFU
#define OldSogdianFirst 0x10F00U
#define OldSogdianLast 0x10F2FU
#define SogdianFirst 0x10F30U
#define SogdianLast 0x10F6FU
#define OldUyghurFirst 0x10F70U
#define OldUyghurLast 0x10FAFU
#define ChorasmianFirst 0x10FB0U
#define ChorasmianLast 0x10FDFU
#define ElymaicFirst 0x10FE0U
#define ElymaicLast 0x10FFFU
#define BrahmiFirst 0x11000U
#define BrahmiLast 0x1107FU
#define KaithiFirst 0x11080U
#define KaithiLast 0x110CFU
#define SoraSompengFirst 0x110D0U
#define SoraSompengLast 0x110FFU
#define ChakmaFirst 0x11100U
#define ChakmaLast 0x1114FU
#define MahajaniFirst 0x11150U
#define MahajaniLast 0x1117FU
#define SharadaFirst 0x11180U
#define SharadaLast 0x111DFU
#define SinhalaArchaicNumbersFirst 0x111E0U
#define SinhalaArchaicNumbersLast 0x111FFU
#define KhojkiFirst 0x11200U
#define KhojkiLast 0x1124FU
#define MultaniFirst 0x11280U
#define MultaniLast 0x112AFU
#define KhudawadiFirst 0x112B0U
#define KhudawadiLast 0x112FFU
#define GranthaFirst 0x11300U
#define GranthaLast 0x1137FU
#define TuluTigalariFirst 0x11380U
#define TuluTigalariLast 0x113FFU
#define NewaFirst 0x11400U
#define NewaLast 0x1147FU
#define TirhutaFirst 0x11480U
#define TirhutaLast 0x114DFU
#define SiddhamFirst 0x11580U
#define SiddhamLast 0x115FFU
#define ModiFirst 0x11600U
#define ModiLast 0x1165FU
#define MongolianSupplementFirst 0x11660U
#define MongolianSupplementLast 0x1167FU
#define TakriFirst 0x11680U
#define TakriLast 0x116CFU
#define MyanmarExtendedCFirst 0x116D0U
#define MyanmarExtendedCLast 0x116FFU
#define AhomFirst 0x11700U
#define AhomLast 0x1174FU
#define DograFirst 0x11800U
#define DograLast 0x1184FU
#define WarangCitiFirst 0x118A0U
#define WarangCitiLast 0x118FFU
#define DivesAkuruFirst 0x11900U
#define DivesAkuruLast 0x1195FU
#define NandinagariFirst 0x119A0U
#define NandinagariLast 0x119FFU
#define ZanabazarSquareFirst 0x11A00U
#define ZanabazarSquareLast 0x11A4FU
#define SoyomboFirst 0x11A50U
#define SoyomboLast 0x11AAFU
#define UnifiedCanadianAboriginalSyllabicsExtendedAFirst 0x11AB0U
#define UnifiedCanadianAboriginalSyllabicsExtendedALast 0x11ABFU
#define PauCinHauFirst 0x11AC0U
#define PauCinHauLast 0x11AFFU
#define DevanagariExtendedAFirst 0x11B00U
#define DevanagariExtendedALast 0x11B5FU
#define SharadaSupplementFirst 0x11B60U
#define SharadaSupplementLast 0x11B7FU
#define SunuwarFirst 0x11BC0U
#define SunuwarLast 0x11BFFU
#define BhaiksukiFirst 0x11C00U
#define BhaiksukiLast 0x11C6FU
#define MarchenFirst 0x11C70U
#define MarchenLast 0x11CBFU
#define MasaramGondiFirst 0x11D00U
#define MasaramGondiLast 0x11D5FU
#define GunjalaGondiFirst 0x11D60U
#define GunjalaGondiLast 0x11DAFU
#define TolongSikiFirst 0x11DB0U
#define TolongSikiLast 0x11DEFU
#define BengaliSupplementFirst 0x11DF0U
#define BengaliSupplementLast 0x11DFFU
#define MakasarFirst 0x11EE0U
#define MakasarLast 0x11EFFU
#define KawiFirst 0x11F00U
#define KawiLast 0x11F5FU
#define LisuSupplementFirst 0x11FB0U
#define LisuSupplementLast 0x11FBFU
#define TamilSupplementFirst 0x11FC0U
#define TamilSupplementLast 0x11FFFU
#define CuneiformFirst 0x12000U
#define CuneiformLast 0x123FFU
#define CuneiformNumbersandPunctuationFirst 0x12400U
#define CuneiformNumbersandPunctuationLast 0x1247FU
#define EarlyDynasticCuneiformFirst 0x12480U
#define EarlyDynasticCuneiformLast 0x1254FU
#define ArchaicCuneiformNumeralsFirst 0x12550U
#define ArchaicCuneiformNumeralsLast 0x1268FU
#define CyproMinoanFirst 0x12F90U
#define CyproMinoanLast 0x12FFFU
#define EgyptianHieroglyphsFirst 0x13000U
#define EgyptianHieroglyphsLast 0x1342FU
#define EgyptianHieroglyphFormatControlsFirst 0x13430U
#define EgyptianHieroglyphFormatControlsLast 0x1345FU
#define EgyptianHieroglyphsExtendedAFirst 0x13460U
#define EgyptianHieroglyphsExtendedALast 0x143FFU
#define AnatolianHieroglyphsFirst 0x14400U
#define AnatolianHieroglyphsLast 0x1467FU
#define GurungKhemaFirst 0x16100U
#define GurungKhemaLast 0x1613FU
#define BamumSupplementFirst 0x16800U
#define BamumSupplementLast 0x16A3FU
#define MroFirst 0x16A40U
#define MroLast 0x16A6FU
#define TangsaFirst 0x16A70U
#define TangsaLast 0x16ACFU
#define BassaVahFirst 0x16AD0U
#define BassaVahLast 0x16AFFU
#define PahawhHmongFirst 0x16B00U
#define PahawhHmongLast 0x16B8FU
#define KiratRaiFirst 0x16D40U
#define KiratRaiLast 0x16D7FU
#define MedefaidrinFirst 0x16E40U
#define MedefaidrinLast 0x16E9FU
#define BeriaErfeFirst 0x16EA0U
#define BeriaErfeLast 0x16EDFU
#define MiaoFirst 0x16F00U
#define MiaoLast 0x16F9FU
#define IdeographicSymbolsandPunctuationFirst 0x16FE0U
#define IdeographicSymbolsandPunctuationLast 0x16FFFU
#define TangutFirst 0x17000U
#define TangutLast 0x187FFU
#define TangutComponentsFirst 0x18800U
#define TangutComponentsLast 0x18AFFU
#define KhitanSmallScriptFirst 0x18B00U
#define KhitanSmallScriptLast 0x18CFFU
#define TangutSupplementFirst 0x18D00U
#define TangutSupplementLast 0x18D7FU
#define TangutComponentsSupplementFirst 0x18D80U
#define TangutComponentsSupplementLast 0x18DFFU
#define JurchenFirst 0x18E00U
#define JurchenLast 0x1919FU
#define JurchenRadicalsFirst 0x191A0U
#define JurchenRadicalsLast 0x191DFU
#define KanaExtendedBFirst 0x1AFF0U
#define KanaExtendedBLast 0x1AFFFU
#define KanaSupplementFirst 0x1B000U
#define KanaSupplementLast 0x1B0FFU
#define KanaExtendedAFirst 0x1B100U
#define KanaExtendedALast 0x1B12FU
#define SmallKanaExtensionFirst 0x1B130U
#define SmallKanaExtensionLast 0x1B16FU
#define NushuFirst 0x1B170U
#define NushuLast 0x1B2FFU
#define DuployanFirst 0x1BC00U
#define DuployanLast 0x1BC9FU
#define ShorthandFormatControlsFirst 0x1BCA0U
#define ShorthandFormatControlsLast 0x1BCAFU
#define SymbolsforLegacyComputingSupplementFirst 0x1CC00U
#define SymbolsforLegacyComputingSupplementLast 0x1CEBFU
#define MiscellaneousSymbolsSupplementFirst 0x1CEC0U
#define MiscellaneousSymbolsSupplementLast 0x1CEFFU
#define ZnamennyMusicalNotationFirst 0x1CF00U
#define ZnamennyMusicalNotationLast 0x1CFCFU
#define ByzantineMusicalSymbolsFirst 0x1D000U
#define ByzantineMusicalSymbolsLast 0x1D0FFU
#define MusicalSymbolsFirst 0x1D100U
#define MusicalSymbolsLast 0x1D1FFU
#define AncientGreekMusicalNotationFirst 0x1D200U
#define AncientGreekMusicalNotationLast 0x1D24FU
#define MusicalSymbolsSupplementFirst 0x1D250U
#define MusicalSymbolsSupplementLast 0x1D28FU
#define KaktovikNumeralsFirst 0x1D2C0U
#define KaktovikNumeralsLast 0x1D2DFU
#define MayanNumeralsFirst 0x1D2E0U
#define MayanNumeralsLast 0x1D2FFU
#define TaiXuanJingSymbolsFirst 0x1D300U
#define TaiXuanJingSymbolsLast 0x1D35FU
#define CountingRodNumeralsFirst 0x1D360U
#define CountingRodNumeralsLast 0x1D37FU
#define MathematicalAlphanumericSymbolsFirst 0x1D400U
#define MathematicalAlphanumericSymbolsLast 0x1D7FFU
#define SuttonSignWritingFirst 0x1D800U
#define SuttonSignWritingLast 0x1DAAFU
#define MiscellaneousSymbolsandArrowsExtendedFirst 0x1DB00U
#define MiscellaneousSymbolsandArrowsExtendedLast 0x1DBFFU
#define LatinExtendedGFirst 0x1DF00U
#define LatinExtendedGLast 0x1DFFFU
#define GlagoliticSupplementFirst 0x1E000U
#define GlagoliticSupplementLast 0x1E02FU
#define CyrillicExtendedDFirst 0x1E030U
#define CyrillicExtendedDLast 0x1E08FU
#define NyiakengPuachueHmongFirst 0x1E100U
#define NyiakengPuachueHmongLast 0x1E14FU
#define TotoFirst 0x1E290U
#define TotoLast 0x1E2BFU
#define WanchoFirst 0x1E2C0U
#define WanchoLast 0x1E2FFU
#define NagMundariFirst 0x1E4D0U
#define NagMundariLast 0x1E4FFU
#define OlOnalFirst 0x1E5D0U
#define OlOnalLast 0x1E5FFU
#define TaiYoFirst 0x1E6C0U
#define TaiYoLast 0x1E6FFU
#define EthiopicExtendedBFirst 0x1E7E0U
#define EthiopicExtendedBLast 0x1E7FFU
#define MendeKikakuiFirst 0x1E800U
#define MendeKikakuiLast 0x1E8DFU
#define AdlamFirst 0x1E900U
#define AdlamLast 0x1E95FU
#define IndicSiyaqNumbersFirst 0x1EC70U
#define IndicSiyaqNumbersLast 0x1ECBFU
#define OttomanSiyaqNumbersFirst 0x1ED00U
#define OttomanSiyaqNumbersLast 0x1ED4FU
#define ArabicMathematicalAlphabeticSymbolsFirst 0x1EE00U
#define ArabicMathematicalAlphabeticSymbolsLast 0x1EEFFU
#define MahjongTilesFirst 0x1F000U
#define MahjongTilesLast 0x1F02FU
#define DominoTilesFirst 0x1F030U
#define DominoTilesLast 0x1F09FU
#define PlayingCardsFirst 0x1F0A0U
#define PlayingCardsLast 0x1F0FFU
#define EnclosedAlphanumericSupplementFirst 0x1F100U
#define EnclosedAlphanumericSupplementLast 0x1F1FFU
#define EnclosedIdeographicSupplementFirst 0x1F200U
#define EnclosedIdeographicSupplementLast 0x1F2FFU
#define MiscellaneousSymbolsAndPictographsFirst 0x1F300U
#define MiscellaneousSymbolsAndPictographsLast 0x1F5FFU
#define EmoticonsFirst 0x1F600U
#define EmoticonsLast 0x1F64FU
#define OrnamentalDingbatsFirst 0x1F650U
#define OrnamentalDingbatsLast 0x1F67FU
#define TransportAndMapSymbolsFirst 0x1F680U
#define TransportAndMapSymbolsLast 0x1F6FFU
#define AlchemicalSymbolsFirst 0x1F700U
#define AlchemicalSymbolsLast 0x1F77FU
#define GeometricShapesExtendedFirst 0x1F780U
#define GeometricShapesExtendedLast 0x1F7FFU
#define SupplementalArrowsCFirst 0x1F800U
#define SupplementalArrowsCLast 0x1F8FFU
#define SupplementalSymbolsandPictographsFirst 0x1F900U
#define SupplementalSymbolsandPictographsLast 0x1F9FFU
#define ChessSymbolsFirst 0x1FA00U
#define ChessSymbolsLast 0x1FA6FU
#define SymbolsandPictographsExtendedAFirst 0x1FA70U
#define SymbolsandPictographsExtendedALast 0x1FAFFU
#define SymbolsforLegacyComputingFirst 0x1FB00U
#define SymbolsforLegacyComputingLast 0x1FBFFU
#define CJKUnifiedIdeographsExtensionBFirst 0x20000U
#define CJKUnifiedIdeographsExtensionBLast 0x2A6DFU
#define CJKUnifiedIdeographsExtensionCFirst 0x2A700U
#define CJKUnifiedIdeographsExtensionCLast 0x2B73FU
#define CJKUnifiedIdeographsExtensionDFirst 0x2B740U
#define CJKUnifiedIdeographsExtensionDLast 0x2B81FU
#define CJKUnifiedIdeographsExtensionEFirst 0x2B820U
#define CJKUnifiedIdeographsExtensionELast 0x2CEAFU
#define CJKUnifiedIdeographsExtensionFFirst 0x2CEB0U
#define CJKUnifiedIdeographsExtensionFLast 0x2EBEFU
#define CJKUnifiedIdeographsExtensionIFirst 0x2EBF0U
#define CJKUnifiedIdeographsExtensionILast 0x2EE5FU
#define CJKCompatibilityIdeographsSupplementFirst 0x2F800U
#define CJKCompatibilityIdeographsSupplementLast 0x2FA1FU
#define CJKUnifiedIdeographsExtensionGFirst 0x30000U
#define CJKUnifiedIdeographsExtensionGLast 0x3134FU
#define CJKUnifiedIdeographsExtensionHFirst 0x31350U
#define CJKUnifiedIdeographsExtensionHLast 0x323AFU
#define CJKUnifiedIdeographsExtensionJFirst 0x323B0U
#define CJKUnifiedIdeographsExtensionJLast 0x3347FU
#define SealFirst 0x3D000U
#define SealLast 0x3FC3FU
#define TagsFirst 0xE0000U
#define TagsLast 0xE007FU
#define VariationSelectorsSupplementFirst 0xE0100U
#define VariationSelectorsSupplementLast 0xE01EFU
#define SupplementaryPrivateUseAreaAFirst 0xF0000U
#define SupplementaryPrivateUseAreaALast 0xFFFFFU
#define SupplementaryPrivateUseAreaBFirst 0x100000U
#define SupplementaryPrivateUseAreaBLast 0x10FFFFU

static constexpr std::array<std::pair<u32, u32>, 353> UnicodeBlocks = 
{
    std::pair{ BasicLatinFirst, BasicLatinLast },
    std::pair{ Latin1SupplementFirst, Latin1SupplementLast },
    std::pair{ LatinExtendedAFirst, LatinExtendedALast },
    std::pair{ LatinExtendedBFirst, LatinExtendedBLast },
    std::pair{ IPAExtensionsFirst, IPAExtensionsLast },
    std::pair{ SpacingModifierLettersFirst, SpacingModifierLettersLast },
    std::pair{ CombiningDiacriticalMarksFirst, CombiningDiacriticalMarksLast },
    std::pair{ GreekandCopticFirst, GreekandCopticLast },
    std::pair{ CyrillicFirst, CyrillicLast },
    std::pair{ CyrillicSupplementFirst, CyrillicSupplementLast },
    std::pair{ ArmenianFirst, ArmenianLast },
    std::pair{ HebrewFirst, HebrewLast },
    std::pair{ ArabicFirst, ArabicLast },
    std::pair{ SyriacFirst, SyriacLast },
    std::pair{ ArabicSupplementFirst, ArabicSupplementLast },
    std::pair{ ThaanaFirst, ThaanaLast },
    std::pair{ NKoFirst, NKoLast },
    std::pair{ SamaritanFirst, SamaritanLast },
    std::pair{ MandaicFirst, MandaicLast },
    std::pair{ SyriacSupplementFirst, SyriacSupplementLast },
    std::pair{ ArabicExtendedBFirst, ArabicExtendedBLast },
    std::pair{ ArabicExtendedAFirst, ArabicExtendedALast },
    std::pair{ DevanagariFirst, DevanagariLast },
    std::pair{ BengaliFirst, BengaliLast },
    std::pair{ GurmukhiFirst, GurmukhiLast },
    std::pair{ GujaratiFirst, GujaratiLast },
    std::pair{ OriyaFirst, OriyaLast },
    std::pair{ TamilFirst, TamilLast },
    std::pair{ TeluguFirst, TeluguLast },
    std::pair{ KannadaFirst, KannadaLast },
    std::pair{ MalayalamFirst, MalayalamLast },
    std::pair{ SinhalaFirst, SinhalaLast },
    std::pair{ ThaiFirst, ThaiLast },
    std::pair{ LaoFirst, LaoLast },
    std::pair{ TibetanFirst, TibetanLast },
    std::pair{ MyanmarFirst, MyanmarLast },
    std::pair{ GeorgianFirst, GeorgianLast },
    std::pair{ HangulJamoFirst, HangulJamoLast },
    std::pair{ EthiopicFirst, EthiopicLast },
    std::pair{ EthiopicSupplementFirst, EthiopicSupplementLast },
    std::pair{ CherokeeFirst, CherokeeLast },
    std::pair{ UnifiedCanadianAboriginalSyllabicsFirst, UnifiedCanadianAboriginalSyllabicsLast },
    std::pair{ OghamFirst, OghamLast },
    std::pair{ RunicFirst, RunicLast },
    std::pair{ TagalogFirst, TagalogLast },
    std::pair{ HanunooFirst, HanunooLast },
    std::pair{ BuhidFirst, BuhidLast },
    std::pair{ TagbanwaFirst, TagbanwaLast },
    std::pair{ KhmerFirst, KhmerLast },
    std::pair{ MongolianFirst, MongolianLast },
    std::pair{ UnifiedCanadianAboriginalSyllabicsExtendedFirst, UnifiedCanadianAboriginalSyllabicsExtendedLast },
    std::pair{ LimbuFirst, LimbuLast },
    std::pair{ TaiLeFirst, TaiLeLast },
    std::pair{ NewTaiLueFirst, NewTaiLueLast },
    std::pair{ KhmerSymbolsFirst, KhmerSymbolsLast },
    std::pair{ BugineseFirst, BugineseLast },
    std::pair{ TaiThamFirst, TaiThamLast },
    std::pair{ CombiningDiacriticalMarksExtendedFirst, CombiningDiacriticalMarksExtendedLast },
    std::pair{ BalineseFirst, BalineseLast },
    std::pair{ SundaneseFirst, SundaneseLast },
    std::pair{ BatakFirst, BatakLast },
    std::pair{ LepchaFirst, LepchaLast },
    std::pair{ OlChikiFirst, OlChikiLast },
    std::pair{ CyrillicExtendedCFirst, CyrillicExtendedCLast },
    std::pair{ GeorgianExtendedFirst, GeorgianExtendedLast },
    std::pair{ SundaneseSupplementFirst, SundaneseSupplementLast },
    std::pair{ VedicExtensionsFirst, VedicExtensionsLast },
    std::pair{ PhoneticExtensionsFirst, PhoneticExtensionsLast },
    std::pair{ PhoneticExtensionsSupplementFirst, PhoneticExtensionsSupplementLast },
    std::pair{ CombiningDiacriticalMarksSupplementFirst, CombiningDiacriticalMarksSupplementLast },
    std::pair{ LatinExtendedAdditionalFirst, LatinExtendedAdditionalLast },
    std::pair{ GreekExtendedFirst, GreekExtendedLast },
    std::pair{ GeneralPunctuationFirst, GeneralPunctuationLast },
    std::pair{ SuperscriptsandSubscriptsFirst, SuperscriptsandSubscriptsLast },
    std::pair{ CurrencySymbolsFirst, CurrencySymbolsLast },
    std::pair{ CombiningDiacriticalMarksforSymbolsFirst, CombiningDiacriticalMarksforSymbolsLast },
    std::pair{ LetterlikeSymbolsFirst, LetterlikeSymbolsLast },
    std::pair{ NumberFormsFirst, NumberFormsLast },
    std::pair{ ArrowsFirst, ArrowsLast },
    std::pair{ MathematicalOperatorsFirst, MathematicalOperatorsLast },
    std::pair{ MiscellaneousTechnicalFirst, MiscellaneousTechnicalLast },
    std::pair{ ControlPicturesFirst, ControlPicturesLast },
    std::pair{ OpticalCharacterRecognitionFirst, OpticalCharacterRecognitionLast },
    std::pair{ EnclosedAlphanumericsFirst, EnclosedAlphanumericsLast },
    std::pair{ BoxDrawingFirst, BoxDrawingLast },
    std::pair{ BlockElementsFirst, BlockElementsLast },
    std::pair{ GeometricShapesFirst, GeometricShapesLast },
    std::pair{ MiscellaneousSymbolsFirst, MiscellaneousSymbolsLast },
    std::pair{ DingbatsFirst, DingbatsLast },
    std::pair{ MiscellaneousMathematicalSymbolsAFirst, MiscellaneousMathematicalSymbolsALast },
    std::pair{ SupplementalArrowsAFirst, SupplementalArrowsALast },
    std::pair{ BraillePatternsFirst, BraillePatternsLast },
    std::pair{ SupplementalArrowsBFirst, SupplementalArrowsBLast },
    std::pair{ MiscellaneousMathematicalSymbolsBFirst, MiscellaneousMathematicalSymbolsBLast },
    std::pair{ SupplementalMathematicalOperatorsFirst, SupplementalMathematicalOperatorsLast },
    std::pair{ MiscellaneousSymbolsandArrowsFirst, MiscellaneousSymbolsandArrowsLast },
    std::pair{ GlagoliticFirst, GlagoliticLast },
    std::pair{ LatinExtendedCFirst, LatinExtendedCLast },
    std::pair{ CopticFirst, CopticLast },
    std::pair{ GeorgianSupplementFirst, GeorgianSupplementLast },
    std::pair{ TifinaghFirst, TifinaghLast },
    std::pair{ EthiopicExtendedFirst, EthiopicExtendedLast },
    std::pair{ CyrillicExtendedAFirst, CyrillicExtendedALast },
    std::pair{ SupplementalPunctuationFirst, SupplementalPunctuationLast },
    std::pair{ CJKRadicalsSupplementFirst, CJKRadicalsSupplementLast },
    std::pair{ KangxiRadicalsFirst, KangxiRadicalsLast },
    std::pair{ IdeographicDescriptionCharactersFirst, IdeographicDescriptionCharactersLast },
    std::pair{ CJKSymbolsandPunctuationFirst, CJKSymbolsandPunctuationLast },
    std::pair{ HiraganaFirst, HiraganaLast },
    std::pair{ KatakanaFirst, KatakanaLast },
    std::pair{ BopomofoFirst, BopomofoLast },
    std::pair{ HangulCompatibilityJamoFirst, HangulCompatibilityJamoLast },
    std::pair{ KanbunFirst, KanbunLast },
    std::pair{ BopomofoExtendedFirst, BopomofoExtendedLast },
    std::pair{ CJKStrokesFirst, CJKStrokesLast },
    std::pair{ KatakanaPhoneticExtensionsFirst, KatakanaPhoneticExtensionsLast },
    std::pair{ EnclosedCJKLettersandMonthsFirst, EnclosedCJKLettersandMonthsLast },
    std::pair{ CJKCompatibilityFirst, CJKCompatibilityLast },
    std::pair{ CJKUnifiedIdeographsExtensionAFirst, CJKUnifiedIdeographsExtensionALast },
    std::pair{ YijingHexagramSymbolsFirst, YijingHexagramSymbolsLast },
    std::pair{ CJKUnifiedIdeographsFirst, CJKUnifiedIdeographsLast },
    std::pair{ YiSyllablesFirst, YiSyllablesLast },
    std::pair{ YiRadicalsFirst, YiRadicalsLast },
    std::pair{ LisuFirst, LisuLast },
    std::pair{ VaiFirst, VaiLast },
    std::pair{ CyrillicExtendedBFirst, CyrillicExtendedBLast },
    std::pair{ BamumFirst, BamumLast },
    std::pair{ ModifierToneLettersFirst, ModifierToneLettersLast },
    std::pair{ LatinExtendedDFirst, LatinExtendedDLast },
    std::pair{ SylotiNagriFirst, SylotiNagriLast },
    std::pair{ CommonIndicNumberFormsFirst, CommonIndicNumberFormsLast },
    std::pair{ PhagspaFirst, PhagspaLast },
    std::pair{ SaurashtraFirst, SaurashtraLast },
    std::pair{ DevanagariExtendedFirst, DevanagariExtendedLast },
    std::pair{ KayahLiFirst, KayahLiLast },
    std::pair{ RejangFirst, RejangLast },
    std::pair{ HangulJamoExtendedAFirst, HangulJamoExtendedALast },
    std::pair{ JavaneseFirst, JavaneseLast },
    std::pair{ MyanmarExtendedBFirst, MyanmarExtendedBLast },
    std::pair{ ChamFirst, ChamLast },
    std::pair{ MyanmarExtendedAFirst, MyanmarExtendedALast },
    std::pair{ TaiVietFirst, TaiVietLast },
    std::pair{ MeeteiMayekExtensionsFirst, MeeteiMayekExtensionsLast },
    std::pair{ EthiopicExtendedAFirst, EthiopicExtendedALast },
    std::pair{ LatinExtendedEFirst, LatinExtendedELast },
    std::pair{ CherokeeSupplementFirst, CherokeeSupplementLast },
    std::pair{ MeeteiMayekFirst, MeeteiMayekLast },
    std::pair{ HangulSyllablesFirst, HangulSyllablesLast },
    std::pair{ HangulJamoExtendedBFirst, HangulJamoExtendedBLast },
    std::pair{ HighSurrogatesFirst, HighSurrogatesLast },
    std::pair{ HighPrivateUseSurrogatesFirst, HighPrivateUseSurrogatesLast },
    std::pair{ LowSurrogatesFirst, LowSurrogatesLast },
    std::pair{ PrivateUseAreaFirst, PrivateUseAreaLast },
    std::pair{ CJKCompatibilityIdeographsFirst, CJKCompatibilityIdeographsLast },
    std::pair{ AlphabeticPresentationFormsFirst, AlphabeticPresentationFormsLast },
    std::pair{ ArabicPresentationFormsAFirst, ArabicPresentationFormsALast },
    std::pair{ VariationSelectorsFirst, VariationSelectorsLast },
    std::pair{ VerticalFormsFirst, VerticalFormsLast },
    std::pair{ CombiningHalfMarksFirst, CombiningHalfMarksLast },
    std::pair{ CJKCompatibilityFormsFirst, CJKCompatibilityFormsLast },
    std::pair{ SmallFormVariantsFirst, SmallFormVariantsLast },
    std::pair{ ArabicPresentationFormsBFirst, ArabicPresentationFormsBLast },
    std::pair{ HalfwidthandFullwidthFormsFirst, HalfwidthandFullwidthFormsLast },
    std::pair{ SpecialsFirst, SpecialsLast },
    std::pair{ LinearBSyllabaryFirst, LinearBSyllabaryLast },
    std::pair{ LinearBIdeogramsFirst, LinearBIdeogramsLast },
    std::pair{ AegeanNumbersFirst, AegeanNumbersLast },
    std::pair{ AncientGreekNumbersFirst, AncientGreekNumbersLast },
    std::pair{ AncientSymbolsFirst, AncientSymbolsLast },
    std::pair{ PhaistosDiscFirst, PhaistosDiscLast },
    std::pair{ LycianFirst, LycianLast },
    std::pair{ CarianFirst, CarianLast },
    std::pair{ CopticEpactNumbersFirst, CopticEpactNumbersLast },
    std::pair{ OldItalicFirst, OldItalicLast },
    std::pair{ GothicFirst, GothicLast },
    std::pair{ OldPermicFirst, OldPermicLast },
    std::pair{ UgariticFirst, UgariticLast },
    std::pair{ OldPersianFirst, OldPersianLast },
    std::pair{ DeseretFirst, DeseretLast },
    std::pair{ ShavianFirst, ShavianLast },
    std::pair{ OsmanyaFirst, OsmanyaLast },
    std::pair{ OsageFirst, OsageLast },
    std::pair{ ElbasanFirst, ElbasanLast },
    std::pair{ CaucasianAlbanianFirst, CaucasianAlbanianLast },
    std::pair{ VithkuqiFirst, VithkuqiLast },
    std::pair{ TodhriFirst, TodhriLast },
    std::pair{ LinearAFirst, LinearALast },
    std::pair{ LatinExtendedFFirst, LatinExtendedFLast },
    std::pair{ CypriotSyllabaryFirst, CypriotSyllabaryLast },
    std::pair{ ImperialAramaicFirst, ImperialAramaicLast },
    std::pair{ PalmyreneFirst, PalmyreneLast },
    std::pair{ NabataeanFirst, NabataeanLast },
    std::pair{ HatranFirst, HatranLast },
    std::pair{ PhoenicianFirst, PhoenicianLast },
    std::pair{ LydianFirst, LydianLast },
    std::pair{ SideticFirst, SideticLast },
    std::pair{ MeroiticHieroglyphsFirst, MeroiticHieroglyphsLast },
    std::pair{ MeroiticCursiveFirst, MeroiticCursiveLast },
    std::pair{ KharoshthiFirst, KharoshthiLast },
    std::pair{ OldSouthArabianFirst, OldSouthArabianLast },
    std::pair{ OldNorthArabianFirst, OldNorthArabianLast },
    std::pair{ ManichaeanFirst, ManichaeanLast },
    std::pair{ AvestanFirst, AvestanLast },
    std::pair{ InscriptionalParthianFirst, InscriptionalParthianLast },
    std::pair{ InscriptionalPahlaviFirst, InscriptionalPahlaviLast },
    std::pair{ PsalterPahlaviFirst, PsalterPahlaviLast },
    std::pair{ OldTurkicFirst, OldTurkicLast },
    std::pair{ OldHungarianFirst, OldHungarianLast },
    std::pair{ HanifiRohingyaFirst, HanifiRohingyaLast },
    std::pair{ GarayFirst, GarayLast },
    std::pair{ RumiNumeralSymbolsFirst, RumiNumeralSymbolsLast },
    std::pair{ YezidiFirst, YezidiLast },
    std::pair{ ArabicExtendedCFirst, ArabicExtendedCLast },
    std::pair{ OldSogdianFirst, OldSogdianLast },
    std::pair{ SogdianFirst, SogdianLast },
    std::pair{ OldUyghurFirst, OldUyghurLast },
    std::pair{ ChorasmianFirst, ChorasmianLast },
    std::pair{ ElymaicFirst, ElymaicLast },
    std::pair{ BrahmiFirst, BrahmiLast },
    std::pair{ KaithiFirst, KaithiLast },
    std::pair{ SoraSompengFirst, SoraSompengLast },
    std::pair{ ChakmaFirst, ChakmaLast },
    std::pair{ MahajaniFirst, MahajaniLast },
    std::pair{ SharadaFirst, SharadaLast },
    std::pair{ SinhalaArchaicNumbersFirst, SinhalaArchaicNumbersLast },
    std::pair{ KhojkiFirst, KhojkiLast },
    std::pair{ MultaniFirst, MultaniLast },
    std::pair{ KhudawadiFirst, KhudawadiLast },
    std::pair{ GranthaFirst, GranthaLast },
    std::pair{ TuluTigalariFirst, TuluTigalariLast },
    std::pair{ NewaFirst, NewaLast },
    std::pair{ TirhutaFirst, TirhutaLast },
    std::pair{ SiddhamFirst, SiddhamLast },
    std::pair{ ModiFirst, ModiLast },
    std::pair{ MongolianSupplementFirst, MongolianSupplementLast },
    std::pair{ TakriFirst, TakriLast },
    std::pair{ MyanmarExtendedCFirst, MyanmarExtendedCLast },
    std::pair{ AhomFirst, AhomLast },
    std::pair{ DograFirst, DograLast },
    std::pair{ WarangCitiFirst, WarangCitiLast },
    std::pair{ DivesAkuruFirst, DivesAkuruLast },
    std::pair{ NandinagariFirst, NandinagariLast },
    std::pair{ ZanabazarSquareFirst, ZanabazarSquareLast },
    std::pair{ SoyomboFirst, SoyomboLast },
    std::pair{ UnifiedCanadianAboriginalSyllabicsExtendedAFirst, UnifiedCanadianAboriginalSyllabicsExtendedALast },
    std::pair{ PauCinHauFirst, PauCinHauLast },
    std::pair{ DevanagariExtendedAFirst, DevanagariExtendedALast },
    std::pair{ SharadaSupplementFirst, SharadaSupplementLast },
    std::pair{ SunuwarFirst, SunuwarLast },
    std::pair{ BhaiksukiFirst, BhaiksukiLast },
    std::pair{ MarchenFirst, MarchenLast },
    std::pair{ MasaramGondiFirst, MasaramGondiLast },
    std::pair{ GunjalaGondiFirst, GunjalaGondiLast },
    std::pair{ TolongSikiFirst, TolongSikiLast },
    std::pair{ BengaliSupplementFirst, BengaliSupplementLast },
    std::pair{ MakasarFirst, MakasarLast },
    std::pair{ KawiFirst, KawiLast },
    std::pair{ LisuSupplementFirst, LisuSupplementLast },
    std::pair{ TamilSupplementFirst, TamilSupplementLast },
    std::pair{ CuneiformFirst, CuneiformLast },
    std::pair{ CuneiformNumbersandPunctuationFirst, CuneiformNumbersandPunctuationLast },
    std::pair{ EarlyDynasticCuneiformFirst, EarlyDynasticCuneiformLast },
    std::pair{ ArchaicCuneiformNumeralsFirst, ArchaicCuneiformNumeralsLast },
    std::pair{ CyproMinoanFirst, CyproMinoanLast },
    std::pair{ EgyptianHieroglyphsFirst, EgyptianHieroglyphsLast },
    std::pair{ EgyptianHieroglyphFormatControlsFirst, EgyptianHieroglyphFormatControlsLast },
    std::pair{ EgyptianHieroglyphsExtendedAFirst, EgyptianHieroglyphsExtendedALast },
    std::pair{ AnatolianHieroglyphsFirst, AnatolianHieroglyphsLast },
    std::pair{ GurungKhemaFirst, GurungKhemaLast },
    std::pair{ BamumSupplementFirst, BamumSupplementLast },
    std::pair{ MroFirst, MroLast },
    std::pair{ TangsaFirst, TangsaLast },
    std::pair{ BassaVahFirst, BassaVahLast },
    std::pair{ PahawhHmongFirst, PahawhHmongLast },
    std::pair{ KiratRaiFirst, KiratRaiLast },
    std::pair{ MedefaidrinFirst, MedefaidrinLast },
    std::pair{ BeriaErfeFirst, BeriaErfeLast },
    std::pair{ MiaoFirst, MiaoLast },
    std::pair{ IdeographicSymbolsandPunctuationFirst, IdeographicSymbolsandPunctuationLast },
    std::pair{ TangutFirst, TangutLast },
    std::pair{ TangutComponentsFirst, TangutComponentsLast },
    std::pair{ KhitanSmallScriptFirst, KhitanSmallScriptLast },
    std::pair{ TangutSupplementFirst, TangutSupplementLast },
    std::pair{ TangutComponentsSupplementFirst, TangutComponentsSupplementLast },
    std::pair{ JurchenFirst, JurchenLast },
    std::pair{ JurchenRadicalsFirst, JurchenRadicalsLast },
    std::pair{ KanaExtendedBFirst, KanaExtendedBLast },
    std::pair{ KanaSupplementFirst, KanaSupplementLast },
    std::pair{ KanaExtendedAFirst, KanaExtendedALast },
    std::pair{ SmallKanaExtensionFirst, SmallKanaExtensionLast },
    std::pair{ NushuFirst, NushuLast },
    std::pair{ DuployanFirst, DuployanLast },
    std::pair{ ShorthandFormatControlsFirst, ShorthandFormatControlsLast },
    std::pair{ SymbolsforLegacyComputingSupplementFirst, SymbolsforLegacyComputingSupplementLast },
    std::pair{ MiscellaneousSymbolsSupplementFirst, MiscellaneousSymbolsSupplementLast },
    std::pair{ ZnamennyMusicalNotationFirst, ZnamennyMusicalNotationLast },
    std::pair{ ByzantineMusicalSymbolsFirst, ByzantineMusicalSymbolsLast },
    std::pair{ MusicalSymbolsFirst, MusicalSymbolsLast },
    std::pair{ AncientGreekMusicalNotationFirst, AncientGreekMusicalNotationLast },
    std::pair{ MusicalSymbolsSupplementFirst, MusicalSymbolsSupplementLast },
    std::pair{ KaktovikNumeralsFirst, KaktovikNumeralsLast },
    std::pair{ MayanNumeralsFirst, MayanNumeralsLast },
    std::pair{ TaiXuanJingSymbolsFirst, TaiXuanJingSymbolsLast },
    std::pair{ CountingRodNumeralsFirst, CountingRodNumeralsLast },
    std::pair{ MathematicalAlphanumericSymbolsFirst, MathematicalAlphanumericSymbolsLast },
    std::pair{ SuttonSignWritingFirst, SuttonSignWritingLast },
    std::pair{ MiscellaneousSymbolsandArrowsExtendedFirst, MiscellaneousSymbolsandArrowsExtendedLast },
    std::pair{ LatinExtendedGFirst, LatinExtendedGLast },
    std::pair{ GlagoliticSupplementFirst, GlagoliticSupplementLast },
    std::pair{ CyrillicExtendedDFirst, CyrillicExtendedDLast },
    std::pair{ NyiakengPuachueHmongFirst, NyiakengPuachueHmongLast },
    std::pair{ TotoFirst, TotoLast },
    std::pair{ WanchoFirst, WanchoLast },
    std::pair{ NagMundariFirst, NagMundariLast },
    std::pair{ OlOnalFirst, OlOnalLast },
    std::pair{ TaiYoFirst, TaiYoLast },
    std::pair{ EthiopicExtendedBFirst, EthiopicExtendedBLast },
    std::pair{ MendeKikakuiFirst, MendeKikakuiLast },
    std::pair{ AdlamFirst, AdlamLast },
    std::pair{ IndicSiyaqNumbersFirst, IndicSiyaqNumbersLast },
    std::pair{ OttomanSiyaqNumbersFirst, OttomanSiyaqNumbersLast },
    std::pair{ ArabicMathematicalAlphabeticSymbolsFirst, ArabicMathematicalAlphabeticSymbolsLast },
    std::pair{ MahjongTilesFirst, MahjongTilesLast },
    std::pair{ DominoTilesFirst, DominoTilesLast },
    std::pair{ PlayingCardsFirst, PlayingCardsLast },
    std::pair{ EnclosedAlphanumericSupplementFirst, EnclosedAlphanumericSupplementLast },
    std::pair{ EnclosedIdeographicSupplementFirst, EnclosedIdeographicSupplementLast },
    std::pair{ MiscellaneousSymbolsAndPictographsFirst, MiscellaneousSymbolsAndPictographsLast },
    std::pair{ EmoticonsFirst, EmoticonsLast },
    std::pair{ OrnamentalDingbatsFirst, OrnamentalDingbatsLast },
    std::pair{ TransportAndMapSymbolsFirst, TransportAndMapSymbolsLast },
    std::pair{ AlchemicalSymbolsFirst, AlchemicalSymbolsLast },
    std::pair{ GeometricShapesExtendedFirst, GeometricShapesExtendedLast },
    std::pair{ SupplementalArrowsCFirst, SupplementalArrowsCLast },
    std::pair{ SupplementalSymbolsandPictographsFirst, SupplementalSymbolsandPictographsLast },
    std::pair{ ChessSymbolsFirst, ChessSymbolsLast },
    std::pair{ SymbolsandPictographsExtendedAFirst, SymbolsandPictographsExtendedALast },
    std::pair{ SymbolsforLegacyComputingFirst, SymbolsforLegacyComputingLast },
    std::pair{ CJKUnifiedIdeographsExtensionBFirst, CJKUnifiedIdeographsExtensionBLast },
    std::pair{ CJKUnifiedIdeographsExtensionCFirst, CJKUnifiedIdeographsExtensionCLast },
    std::pair{ CJKUnifiedIdeographsExtensionDFirst, CJKUnifiedIdeographsExtensionDLast },
    std::pair{ CJKUnifiedIdeographsExtensionEFirst, CJKUnifiedIdeographsExtensionELast },
    std::pair{ CJKUnifiedIdeographsExtensionFFirst, CJKUnifiedIdeographsExtensionFLast },
    std::pair{ CJKUnifiedIdeographsExtensionIFirst, CJKUnifiedIdeographsExtensionILast },
    std::pair{ CJKCompatibilityIdeographsSupplementFirst, CJKCompatibilityIdeographsSupplementLast },
    std::pair{ CJKUnifiedIdeographsExtensionGFirst, CJKUnifiedIdeographsExtensionGLast },
    std::pair{ CJKUnifiedIdeographsExtensionHFirst, CJKUnifiedIdeographsExtensionHLast },
    std::pair{ CJKUnifiedIdeographsExtensionJFirst, CJKUnifiedIdeographsExtensionJLast },
    std::pair{ SealFirst, SealLast },
    std::pair{ TagsFirst, TagsLast },
    std::pair{ VariationSelectorsSupplementFirst, VariationSelectorsSupplementLast },
    std::pair{ SupplementaryPrivateUseAreaAFirst, SupplementaryPrivateUseAreaALast },
    std::pair{ SupplementaryPrivateUseAreaBFirst, SupplementaryPrivateUseAreaBLast }
};
