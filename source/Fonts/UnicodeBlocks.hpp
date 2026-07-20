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
#define CyrillicSupplementLast 0x527U
#define ArmenianFirst 0x531U
#define ArmenianLast 0x58AU
#define HebrewFirst 0x591U
#define HebrewLast 0x5F4U
#define ArabicFirst 0x600U
#define ArabicLast 0x6FFU
#define SyriacFirst 0x700U
#define SyriacLast 0x74FU
#define ArabicSupplementFirst 0x750U
#define ArabicSupplementLast 0x77FU
#define ThaanaFirst 0x780U
#define ThaanaLast 0x7B1U
#define NKoFirst 0x7C0U
#define NKoLast 0x7FAU
#define SamaritanFirst 0x800U
#define SamaritanLast 0x83EU
#define MandaicFirst 0x840U
#define MandaicLast 0x85EU
#define DevanagariFirst 0x900U
#define DevanagariLast 0x97FU
#define BengaliFirst 0x981U
#define BengaliLast 0x9FBU
#define GurmukhiFirst 0xA01U
#define GurmukhiLast 0xA75U
#define GujaratiFirst 0xA81U
#define GujaratiLast 0xAF1U
#define OriyaFirst 0xB01U
#define OriyaLast 0xB77U
#define TamilFirst 0xB82U
#define TamilLast 0xBFAU
#define TeluguFirst 0xC01U
#define TeluguLast 0xC7FU
#define KannadaFirst 0xC82U
#define KannadaLast 0xCF2U
#define MalayalamFirst 0xD02U
#define MalayalamLast 0xD7FU
#define SinhalaFirst 0xD82U
#define SinhalaLast 0xDF4U
#define ThaiFirst 0xE01U
#define ThaiLast 0xE5BU
#define LaoFirst 0xE81U
#define LaoLast 0xEDDU
#define TibetanFirst 0xF00U
#define TibetanLast 0xFDAU
#define MyanmarFirst 0x1000U
#define MyanmarLast 0x109FU
#define GeorgianFirst 0x10A0U
#define GeorgianLast 0x10FCU
#define HangulJamoFirst 0x1100U
#define HangulJamoLast 0x11FFU
#define EthiopicFirst 0x1200U
#define EthiopicLast 0x137CU
#define EthiopicSupplementFirst 0x1380U
#define EthiopicSupplementLast 0x1399U
#define CherokeeFirst 0x13A0U
#define CherokeeLast 0x13F4U
#define UnifiedCanadianAboriginalSyllabicsFirst 0x1400U
#define UnifiedCanadianAboriginalSyllabicsLast 0x167FU
#define OghamFirst 0x1680U
#define OghamLast 0x169CU
#define RunicFirst 0x16A0U
#define RunicLast 0x16F0U
#define TagalogFirst 0x1700U
#define TagalogLast 0x1714U
#define HanunooFirst 0x1720U
#define HanunooLast 0x1736U
#define BuhidFirst 0x1740U
#define BuhidLast 0x1753U
#define TagbanwaFirst 0x1760U
#define TagbanwaLast 0x1773U
#define KhmerFirst 0x1780U
#define KhmerLast 0x17F9U
#define MongolianFirst 0x1800U
#define MongolianLast 0x18AAU
#define UnifiedCanadianAboriginalSyllabicsExtendedFirst 0x18B0U
#define UnifiedCanadianAboriginalSyllabicsExtendedLast 0x18F5U
#define LimbuFirst 0x1900U
#define LimbuLast 0x194FU
#define TaiLeFirst 0x1950U
#define TaiLeLast 0x1974U
#define NewTaiLueFirst 0x1980U
#define NewTaiLueLast 0x19DFU
#define KhmerSymbolsFirst 0x19E0U
#define KhmerSymbolsLast 0x19FFU
#define BugineseFirst 0x1A00U
#define BugineseLast 0x1A1FU
#define TaiThamFirst 0x1A20U
#define TaiThamLast 0x1AADU
#define BalineseFirst 0x1B00U
#define BalineseLast 0x1B7CU
#define SundaneseFirst 0x1B80U
#define SundaneseLast 0x1BB9U
#define BatakFirst 0x1BC0U
#define BatakLast 0x1BFFU
#define LepchaFirst 0x1C00U
#define LepchaLast 0x1C4FU
#define OlChikiFirst 0x1C50U
#define OlChikiLast 0x1C7FU
#define VedicExtensionsFirst 0x1CD0U
#define VedicExtensionsLast 0x1CF2U
#define PhoneticExtensionsFirst 0x1D00U
#define PhoneticExtensionsLast 0x1D7FU
#define PhoneticExtensionsSupplementFirst 0x1D80U
#define PhoneticExtensionsSupplementLast 0x1DBFU
#define CombiningDiacriticalMarksSupplementFirst 0x1DC0U
#define CombiningDiacriticalMarksSupplementLast 0x1DFFU
#define LatinExtendedAdditionalFirst 0x1E00U
#define LatinExtendedAdditionalLast 0x1EFFU
#define GreekExtendedFirst 0x1F00U
#define GreekExtendedLast 0x1FFEU
#define GeneralPunctuationFirst 0x2000U
#define GeneralPunctuationLast 0x206FU
#define SuperscriptsandSubscriptsFirst 0x2070U
#define SuperscriptsandSubscriptsLast 0x209CU
#define CurrencySymbolsFirst 0x20A0U
#define CurrencySymbolsLast 0x20B9U
#define CombiningDiacriticalMarksforSymbolsFirst 0x20D0U
#define CombiningDiacriticalMarksforSymbolsLast 0x20F0U
#define LetterlikeSymbolsFirst 0x2100U
#define LetterlikeSymbolsLast 0x214FU
#define NumberFormsFirst 0x2150U
#define NumberFormsLast 0x2189U
#define ArrowsFirst 0x2190U
#define ArrowsLast 0x21FFU
#define MathematicalOperatorsFirst 0x2200U
#define MathematicalOperatorsLast 0x22FFU
#define MiscellaneousTechnicalFirst 0x2300U
#define MiscellaneousTechnicalLast 0x23F3U
#define ControlPicturesFirst 0x2400U
#define ControlPicturesLast 0x2426U
#define OpticalCharacterRecognitionFirst 0x2440U
#define OpticalCharacterRecognitionLast 0x244AU
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
#define DingbatsFirst 0x2701U
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
#define MiscellaneousSymbolsandArrowsLast 0x2B59U
#define GlagoliticFirst 0x2C00U
#define GlagoliticLast 0x2C5EU
#define LatinExtendedCFirst 0x2C60U
#define LatinExtendedCLast 0x2C7FU
#define CopticFirst 0x2C80U
#define CopticLast 0x2CFFU
#define GeorgianSupplementFirst 0x2D00U
#define GeorgianSupplementLast 0x2D25U
#define TifinaghFirst 0x2D30U
#define TifinaghLast 0x2D7FU
#define EthiopicExtendedFirst 0x2D80U
#define EthiopicExtendedLast 0x2DDEU
#define CyrillicExtendedAFirst 0x2DE0U
#define CyrillicExtendedALast 0x2DFFU
#define SupplementalPunctuationFirst 0x2E00U
#define SupplementalPunctuationLast 0x2E31U
#define CJKRadicalsSupplementFirst 0x2E80U
#define CJKRadicalsSupplementLast 0x2EF3U
#define KangxiRadicalsFirst 0x2F00U
#define KangxiRadicalsLast 0x2FD5U
#define IdeographicDescriptionCharactersFirst 0x2FF0U
#define IdeographicDescriptionCharactersLast 0x2FFBU
#define CJKSymbolsandPunctuationFirst 0x3000U
#define CJKSymbolsandPunctuationLast 0x303FU
#define HiraganaFirst 0x3041U
#define HiraganaLast 0x309FU
#define KatakanaFirst 0x30A0U
#define KatakanaLast 0x30FFU
#define BopomofoFirst 0x3105U
#define BopomofoLast 0x312DU
#define HangulCompatibilityJamoFirst 0x3131U
#define HangulCompatibilityJamoLast 0x318EU
#define KanbunFirst 0x3190U
#define KanbunLast 0x319FU
#define BopomofoExtendedFirst 0x31A0U
#define BopomofoExtendedLast 0x31BAU
#define CJKStrokesFirst 0x31C0U
#define CJKStrokesLast 0x31E3U
#define KatakanaPhoneticExtensionsFirst 0x31F0U
#define KatakanaPhoneticExtensionsLast 0x31FFU
#define EnclosedCJKLettersandMonthsFirst 0x3200U
#define EnclosedCJKLettersandMonthsLast 0x32FEU
#define CJKCompatibilityFirst 0x3300U
#define CJKCompatibilityLast 0x33FFU
#define CJKUnifiedIdeographsExtensionAFirst 0x3400U
#define CJKUnifiedIdeographsExtensionALast 0x4DB5U
#define YijingHexagramSymbolsFirst 0x4DC0U
#define YijingHexagramSymbolsLast 0x4DFFU
#define CJKUnifiedIdeographsFirst 0x4E00U
#define CJKUnifiedIdeographsLast 0x9FCBU
#define YiSyllablesFirst 0xA000U
#define YiSyllablesLast 0xA48CU
#define YiRadicalsFirst 0xA490U
#define YiRadicalsLast 0xA4C6U
#define LisuFirst 0xA4D0U
#define LisuLast 0xA4FFU
#define VaiFirst 0xA500U
#define VaiLast 0xA62BU
#define CyrillicExtendedBFirst 0xA640U
#define CyrillicExtendedBLast 0xA697U
#define BamumFirst 0xA6A0U
#define BamumLast 0xA6F7U
#define ModifierToneLettersFirst 0xA700U
#define ModifierToneLettersLast 0xA71FU
#define LatinExtendedDFirst 0xA720U
#define LatinExtendedDLast 0xA7FFU
#define SylotiNagriFirst 0xA800U
#define SylotiNagriLast 0xA82BU
#define CommonIndicNumberFormsFirst 0xA830U
#define CommonIndicNumberFormsLast 0xA839U
#define PhagspaFirst 0xA840U
#define PhagspaLast 0xA877U
#define SaurashtraFirst 0xA880U
#define SaurashtraLast 0xA8D9U
#define DevanagariExtendedFirst 0xA8E0U
#define DevanagariExtendedLast 0xA8FBU
#define KayahLiFirst 0xA900U
#define KayahLiLast 0xA92FU
#define RejangFirst 0xA930U
#define RejangLast 0xA95FU
#define HangulJamoExtendedAFirst 0xA960U
#define HangulJamoExtendedALast 0xA97CU
#define JavaneseFirst 0xA980U
#define JavaneseLast 0xA9DFU
#define ChamFirst 0xAA00U
#define ChamLast 0xAA5FU
#define MyanmarExtendedAFirst 0xAA60U
#define MyanmarExtendedALast 0xAA7BU
#define TaiVietFirst 0xAA80U
#define TaiVietLast 0xAADFU
#define EthiopicExtendedAFirst 0xAB01U
#define EthiopicExtendedALast 0xAB2EU
#define MeeteiMayekFirst 0xABC0U
#define MeeteiMayekLast 0xABF9U
#define HangulSyllablesFirst 0xAC00U
#define HangulSyllablesLast 0xD7A3U
#define HangulJamoExtendedBFirst 0xD7B0U
#define HangulJamoExtendedBLast 0xD7FBU
#define HighSurrogatesFirst 0xD800U
#define HighSurrogatesLast 0xDB7FU
#define HighPrivateUseSurrogatesFirst 0xDB80U
#define HighPrivateUseSurrogatesLast 0xDBFFU
#define LowSurrogatesFirst 0xDC00U
#define LowSurrogatesLast 0xDFFFU
#define PrivateUseAreaFirst 0xE000U
#define PrivateUseAreaLast 0xF8FFU
#define CJKCompatibilityIdeographsFirst 0xF900U
#define CJKCompatibilityIdeographsLast 0xFAD9U
#define AlphabeticPresentationFormsFirst 0xFB00U
#define AlphabeticPresentationFormsLast 0xFB4FU
#define ArabicPresentationFormsAFirst 0xFB50U
#define ArabicPresentationFormsALast 0xFDFDU
#define VariationSelectorsFirst 0xFE00U
#define VariationSelectorsLast 0xFE0FU
#define VerticalFormsFirst 0xFE10U
#define VerticalFormsLast 0xFE19U
#define CombiningHalfMarksFirst 0xFE20U
#define CombiningHalfMarksLast 0xFE26U
#define CJKCompatibilityFormsFirst 0xFE30U
#define CJKCompatibilityFormsLast 0xFE4FU
#define SmallFormVariantsFirst 0xFE50U
#define SmallFormVariantsLast 0xFE6BU
#define ArabicPresentationFormsBFirst 0xFE70U
#define ArabicPresentationFormsBLast 0xFEFFU
#define HalfwidthandFullwidthFormsFirst 0xFF01U
#define HalfwidthandFullwidthFormsLast 0xFFEEU
#define SpecialsFirst 0xFFF9U
#define SpecialsLast 0xFFFDU
#define LinearBSyllabaryFirst 0x10000U
#define LinearBSyllabaryLast 0x1005DU
#define LinearBIdeogramsFirst 0x10080U
#define LinearBIdeogramsLast 0x100FAU
#define AegeanNumbersFirst 0x10100U
#define AegeanNumbersLast 0x1013FU
#define AncientGreekNumbersFirst 0x10140U
#define AncientGreekNumbersLast 0x1018AU
#define AncientSymbolsFirst 0x10190U
#define AncientSymbolsLast 0x1019BU
#define PhaistosDiscFirst 0x101D0U
#define PhaistosDiscLast 0x101FDU
#define LycianFirst 0x10280U
#define LycianLast 0x1029CU
#define CarianFirst 0x102A0U
#define CarianLast 0x102D0U
#define OldItalicFirst 0x10300U
#define OldItalicLast 0x10323U
#define GothicFirst 0x10330U
#define GothicLast 0x1034AU
#define UgariticFirst 0x10380U
#define UgariticLast 0x1039FU
#define OldPersianFirst 0x103A0U
#define OldPersianLast 0x103D5U
#define DeseretFirst 0x10400U
#define DeseretLast 0x1044FU
#define ShavianFirst 0x10450U
#define ShavianLast 0x1047FU
#define OsmanyaFirst 0x10480U
#define OsmanyaLast 0x104A9U
#define CypriotSyllabaryFirst 0x10800U
#define CypriotSyllabaryLast 0x1083FU
#define ImperialAramaicFirst 0x10840U
#define ImperialAramaicLast 0x1085FU
#define PhoenicianFirst 0x10900U
#define PhoenicianLast 0x1091FU
#define LydianFirst 0x10920U
#define LydianLast 0x1093FU
#define KharoshthiFirst 0x10A00U
#define KharoshthiLast 0x10A58U
#define OldSouthArabianFirst 0x10A60U
#define OldSouthArabianLast 0x10A7FU
#define AvestanFirst 0x10B00U
#define AvestanLast 0x10B3FU
#define InscriptionalParthianFirst 0x10B40U
#define InscriptionalParthianLast 0x10B5FU
#define InscriptionalPahlaviFirst 0x10B60U
#define InscriptionalPahlaviLast 0x10B7FU
#define OldTurkicFirst 0x10C00U
#define OldTurkicLast 0x10C48U
#define RumiNumeralSymbolsFirst 0x10E60U
#define RumiNumeralSymbolsLast 0x10E7EU
#define BrahmiFirst 0x11000U
#define BrahmiLast 0x1106FU
#define KaithiFirst 0x11080U
#define KaithiLast 0x110C1U
#define CuneiformFirst 0x12000U
#define CuneiformLast 0x1236EU
#define CuneiformNumbersandPunctuationFirst 0x12400U
#define CuneiformNumbersandPunctuationLast 0x12473U
#define EgyptianHieroglyphsFirst 0x13000U
#define EgyptianHieroglyphsLast 0x1342EU
#define BamumSupplementFirst 0x16800U
#define BamumSupplementLast 0x16A38U
#define KanaSupplementFirst 0x1B000U
#define KanaSupplementLast 0x1B001U
#define ByzantineMusicalSymbolsFirst 0x1D000U
#define ByzantineMusicalSymbolsLast 0x1D0F5U
#define MusicalSymbolsFirst 0x1D100U
#define MusicalSymbolsLast 0x1D1DDU
#define AncientGreekMusicalNotationFirst 0x1D200U
#define AncientGreekMusicalNotationLast 0x1D245U
#define TaiXuanJingSymbolsFirst 0x1D300U
#define TaiXuanJingSymbolsLast 0x1D356U
#define CountingRodNumeralsFirst 0x1D360U
#define CountingRodNumeralsLast 0x1D371U
#define MathematicalAlphanumericSymbolsFirst 0x1D400U
#define MathematicalAlphanumericSymbolsLast 0x1D7FFU
#define MahjongTilesFirst 0x1F000U
#define MahjongTilesLast 0x1F02BU
#define DominoTilesFirst 0x1F030U
#define DominoTilesLast 0x1F093U
#define PlayingCardsFirst 0x1F0A0U
#define PlayingCardsLast 0x1F0DFU
#define EnclosedAlphanumericSupplementFirst 0x1F100U
#define EnclosedAlphanumericSupplementLast 0x1F1FFU
#define EnclosedIdeographicSupplementFirst 0x1F200U
#define EnclosedIdeographicSupplementLast 0x1F251U
#define MiscellaneousSymbolsAndPictographsFirst 0x1F300U
#define MiscellaneousSymbolsAndPictographsLast 0x1F5FFU
#define EmoticonsFirst 0x1F600U
#define EmoticonsLast 0x1F64FU
#define TransportAndMapSymbolsFirst 0x1F680U
#define TransportAndMapSymbolsLast 0x1F6C5U
#define AlchemicalSymbolsFirst 0x1F700U
#define AlchemicalSymbolsLast 0x1F773U
#define CJKUnifiedIdeographsExtensionBFirst 0x20000U
#define CJKUnifiedIdeographsExtensionBLast 0x2A6D6U
#define CJKUnifiedIdeographsExtensionCFirst 0x2A700U
#define CJKUnifiedIdeographsExtensionCLast 0x2B734U
#define CJKUnifiedIdeographsExtensionDFirst 0x2B740U
#define CJKUnifiedIdeographsExtensionDLast 0x2B81DU
#define CJKCompatibilityIdeographsSupplementFirst 0x2F800U
#define CJKCompatibilityIdeographsSupplementLast 0x2FA1DU
#define TagsFirst 0xE0001U
#define TagsLast 0xE007FU
#define VariationSelectorsSupplementFirst 0xE0100U
#define VariationSelectorsSupplementLast 0xE01EFU
#define SupplementaryPrivateUseAreaAFirst 0xF0000U
#define SupplementaryPrivateUseAreaALast 0xFFFFDU
#define SupplementaryPrivateUseAreaVFirst 0x100000U
#define SupplementaryPrivateUseAreaVLast 0x10FFFDU

static constexpr std::array<std::pair<u32, u32>, 209> UnicodeBlocks = 
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
    std::pair{ BalineseFirst, BalineseLast },
    std::pair{ SundaneseFirst, SundaneseLast },
    std::pair{ BatakFirst, BatakLast },
    std::pair{ LepchaFirst, LepchaLast },
    std::pair{ OlChikiFirst, OlChikiLast },
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
    std::pair{ ChamFirst, ChamLast },
    std::pair{ MyanmarExtendedAFirst, MyanmarExtendedALast },
    std::pair{ TaiVietFirst, TaiVietLast },
    std::pair{ EthiopicExtendedAFirst, EthiopicExtendedALast },
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
    std::pair{ OldItalicFirst, OldItalicLast },
    std::pair{ GothicFirst, GothicLast },
    std::pair{ UgariticFirst, UgariticLast },
    std::pair{ OldPersianFirst, OldPersianLast },
    std::pair{ DeseretFirst, DeseretLast },
    std::pair{ ShavianFirst, ShavianLast },
    std::pair{ OsmanyaFirst, OsmanyaLast },
    std::pair{ CypriotSyllabaryFirst, CypriotSyllabaryLast },
    std::pair{ ImperialAramaicFirst, ImperialAramaicLast },
    std::pair{ PhoenicianFirst, PhoenicianLast },
    std::pair{ LydianFirst, LydianLast },
    std::pair{ KharoshthiFirst, KharoshthiLast },
    std::pair{ OldSouthArabianFirst, OldSouthArabianLast },
    std::pair{ AvestanFirst, AvestanLast },
    std::pair{ InscriptionalParthianFirst, InscriptionalParthianLast },
    std::pair{ InscriptionalPahlaviFirst, InscriptionalPahlaviLast },
    std::pair{ OldTurkicFirst, OldTurkicLast },
    std::pair{ RumiNumeralSymbolsFirst, RumiNumeralSymbolsLast },
    std::pair{ BrahmiFirst, BrahmiLast },
    std::pair{ KaithiFirst, KaithiLast },
    std::pair{ CuneiformFirst, CuneiformLast },
    std::pair{ CuneiformNumbersandPunctuationFirst, CuneiformNumbersandPunctuationLast },
    std::pair{ EgyptianHieroglyphsFirst, EgyptianHieroglyphsLast },
    std::pair{ BamumSupplementFirst, BamumSupplementLast },
    std::pair{ KanaSupplementFirst, KanaSupplementLast },
    std::pair{ ByzantineMusicalSymbolsFirst, ByzantineMusicalSymbolsLast },
    std::pair{ MusicalSymbolsFirst, MusicalSymbolsLast },
    std::pair{ AncientGreekMusicalNotationFirst, AncientGreekMusicalNotationLast },
    std::pair{ TaiXuanJingSymbolsFirst, TaiXuanJingSymbolsLast },
    std::pair{ CountingRodNumeralsFirst, CountingRodNumeralsLast },
    std::pair{ MathematicalAlphanumericSymbolsFirst, MathematicalAlphanumericSymbolsLast },
    std::pair{ MahjongTilesFirst, MahjongTilesLast },
    std::pair{ DominoTilesFirst, DominoTilesLast },
    std::pair{ PlayingCardsFirst, PlayingCardsLast },
    std::pair{ EnclosedAlphanumericSupplementFirst, EnclosedAlphanumericSupplementLast },
    std::pair{ EnclosedIdeographicSupplementFirst, EnclosedIdeographicSupplementLast },
    std::pair{ MiscellaneousSymbolsAndPictographsFirst, MiscellaneousSymbolsAndPictographsLast },
    std::pair{ EmoticonsFirst, EmoticonsLast },
    std::pair{ TransportAndMapSymbolsFirst, TransportAndMapSymbolsLast },
    std::pair{ AlchemicalSymbolsFirst, AlchemicalSymbolsLast },
    std::pair{ CJKUnifiedIdeographsExtensionBFirst, CJKUnifiedIdeographsExtensionBLast },
    std::pair{ CJKUnifiedIdeographsExtensionCFirst, CJKUnifiedIdeographsExtensionCLast },
    std::pair{ CJKUnifiedIdeographsExtensionDFirst, CJKUnifiedIdeographsExtensionDLast },
    std::pair{ CJKCompatibilityIdeographsSupplementFirst, CJKCompatibilityIdeographsSupplementLast },
    std::pair{ TagsFirst, TagsLast },
    std::pair{ VariationSelectorsSupplementFirst, VariationSelectorsSupplementLast },
    std::pair{ SupplementaryPrivateUseAreaAFirst, SupplementaryPrivateUseAreaALast },
    std::pair{ SupplementaryPrivateUseAreaVFirst, SupplementaryPrivateUseAreaVLast }
};