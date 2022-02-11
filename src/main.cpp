#include <fmt/format.h>
#include <dokkan/files.h>
#include <dokkan/instances.h>
#include <unidokkan/ui/defs.h>
#include <unidokkan/errors.h>
#include <unidokkan/database.h>
#include <unidokkan/ui/label.h>
#include <unidokkan/ui/scene.h>
#include <unidokkan/ui/button.h>
#include <unidokkan/ui/layout.h>
#include <unidokkan/ui/imageview.h>
#include <unidokkan/ui/progresstimer.h>
#include <cocos/ui/UIButton.h>
#include <cocos/base/CCDirector.h>
#include <cocos/2d/CCActionInterval.h>
#include <string>
using namespace UniDokkan::UI;
namespace cocos2d = ud_cocos2d; 
using namespace std::string_view_literals;
using json = nlohmann::json;
using namespace phmap;
const char *CreateIfPossible = "PRAGMA synchronous = OFF;"
"PRAGMA journal_mode = OFF;"
"BEGIN EXCLUSIVE TRANSACTION;"
"PRAGMA defer_foreign_keys = ON;"
"CREATE TABLE IF NOT EXISTS \"index_list\" ("
"	\"id\"	integer NOT NULL PRIMARY KEY AUTOINCREMENT,"
"	\"name\"	varchar(255) NOT NULL"
");"
"COMMIT TRANSACTION;"
"PRAGMA optimize;"
"PRAGMA synchronous = NORMAL;"
"PRAGMA journal_mode = DELETE;"
"PRAGMA shrink_memory;";
const char *collection_categories = "PRAGMA synchronous = OFF;"
"PRAGMA journal_mode = OFF;"
"BEGIN EXCLUSIVE TRANSACTION;"
"PRAGMA defer_foreign_keys = ON;"
"UPDATE collection_categories SET name='ABC' WHERE id=1;"
"UPDATE collection_categories SET name='DEF' WHERE id=2;"
"UPDATE collection_categories SET name='GHI' WHERE id=3;"
"UPDATE collection_categories SET name='JKL' WHERE id=4;"
"UPDATE collection_categories SET name='MNO' WHERE id=5;"
"UPDATE collection_categories SET name='PQR' WHERE id=6;"
"UPDATE collection_categories SET name='STU' WHERE id=7;"
"UPDATE collection_categories SET name='VWX' WHERE id=8;"
"UPDATE collection_categories SET name='YZ' WHERE id=9;"
"COMMIT TRANSACTION;"
"PRAGMA optimize;"
"PRAGMA synchronous = NORMAL;"
"PRAGMA journal_mode = DELETE;"
"PRAGMA shrink_memory;";
constexpr std::string_view kcollection_uniques = R"SQL(
    SELECT
        collection_uniques.id,
        collection_uniques.name
    FROM
        collection_uniques
	ORDER BY
		id
)SQL"sv;
constexpr std::string_view kindex_list = R"SQL(
    SELECT
        index_list.id,
        index_list.name
    FROM
        index_list
	ORDER BY
		name
)SQL"sv;
std::string DetectLanguage(){
	//Language detection : 
	auto file_utils				=	UniDokkan::Dokkan::cocos2d_FileUtils_getInstance();
	auto xmlfile				=	UniDokkan::Dokkan::cocos2d_UserDefault_getInstance();
	static std::string label_ja	=	"sqlite/current/database.db";
	static std::string label_fr	=	"french.txt";
	static std::string label_en	=	"english.txt";
	auto getStringForKey		=	DokkanFunction<std::string(void *,const char *)>("_ZN7cocos2d11UserDefault15getStringForKeyEPKc");
	UD_LOGI("Language detection : let check language");
	if (UniDokkan::Dokkan::cocos2d_FileUtils_isFileExistWithoutCpk(file_utils, label_ja)) {
		if (UniDokkan::Dokkan::cocos2d_FileUtils_isFileExistWithoutCpk(file_utils, label_fr)) {
			UD_LOGI("Language detection : FR Patch found");
			return "fr";
		} else if (UniDokkan::Dokkan::cocos2d_FileUtils_isFileExistWithoutCpk(file_utils, label_en)) {
			UD_LOGI("Language detection : Japan client with english patch");
			return "en";
		} else {
			UD_LOGI("Language detection : Japan client without english patch");
			return "en"; //<- default.
		}
	} else {
		auto glblanguage = getStringForKey(xmlfile, "CurrentUsingLanguage");
		UD_LOGI("Language detection : %s", glblanguage.c_str());
		return glblanguage;
	}
}
void DatabaseReadyCallback() {
	UD_LOGI("IndexGLB : Init");
	auto Lang 			=	DetectLanguage();
	if (Lang == "en" || Lang == "fr") {
		UD_LOGI("IndexGLB : CreateIfPossible - begin");
		UniDokkan::Database::exec(CreateIfPossible);
		UD_LOGI("IndexGLB : CreateIfPossible - ok");
		UD_LOGI("IndexGLB : collection_categories - begin");
		UniDokkan::Database::exec(collection_categories);
		UD_LOGI("IndexGLB : collection_categories - ok");
		UD_LOGI("IndexGLB : kcollection_uniques - begin");
		json collection_list = UniDokkan::Database::select(kcollection_uniques);
		UD_LOGI("IndexGLB : kcollection_uniques - ok");
		std::string InsCols = "PRAGMA synchronous = OFF;"
		"PRAGMA journal_mode = OFF;"
		"BEGIN EXCLUSIVE TRANSACTION;"
		"PRAGMA defer_foreign_keys = ON;";
		for (auto &collection : collection_list) {
			UD_LOGI("collection: %d - %s", collection["id"].get<int>(), collection["name"].get<std::string>().c_str());
			std::string Collection_Name = collection["name"].get<std::string>();
			int Collection_ID 			= collection["id"].get<int>();
			if (Collection_ID == 256){
				Collection_Name = "Yamhan";
			}
			if (Lang == "fr") {
				if (Collection_ID == 431){
					Collection_Name = "C-8";
				}
			}
			InsCols.append("INSERT INTO \"index_list\" (\"id\", \"name\") VALUES (\""+std::to_string(Collection_ID)+"\", \""+Collection_Name+"\");");
		}
		InsCols.append("COMMIT TRANSACTION;");
		InsCols.append("PRAGMA optimize;");
		InsCols.append("PRAGMA synchronous = NORMAL;");
		InsCols.append("PRAGMA journal_mode = DELETE;");
		InsCols.append("PRAGMA shrink_memory;");
		UD_LOGI("IndexGLB : Inserting");
		UniDokkan::Database::exec(InsCols);
		UD_LOGI("IndexGLB : inserted");
		UD_LOGI("IndexGLB : Fixing");
		const char *FixName ="PRAGMA synchronous = OFF;"
		"PRAGMA journal_mode = OFF;"
		"BEGIN EXCLUSIVE TRANSACTION;"
		"PRAGMA defer_foreign_keys = ON;"
		"UPDATE index_list SET name = REPLACE(lower(name), 'á','a');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'ã','a');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'â','a');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'é','e');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'ê','e');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'è','e');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'è','e');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'í','i');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'ó','o');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'ö','o');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'ú','u');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'ç','c');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'É','e');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'À','a');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'Á','a');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'Â','a');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'Ã','a');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'Ä','a');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'Å','a');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'È','e');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'Ê','e');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'Ë','e');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'Î','i');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'Ï','i');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'Ì','i');"
		"UPDATE index_list SET name = REPLACE(lower(name), 'Í','i');"
		"COMMIT TRANSACTION;"
		"PRAGMA optimize;"
		"PRAGMA synchronous = NORMAL;"
		"PRAGMA journal_mode = DELETE;"
		"PRAGMA shrink_memory;";
		UniDokkan::Database::exec(FixName);
		UD_LOGI("IndexGLB : fixed");
		json kindex_list_fixed = UniDokkan::Database::select(kindex_list);
		UD_LOGI("IndexGLB : Listing Fixed:");
		int C_ABC = 5000;
		int C_DEF = 5000;
		int C_GHI = 5000;
		int C_JKL = 5000;
		int C_MNO = 5000;
		int C_PQR = 5000;
		int C_STU = 5000;
		int C_VWX = 5000;
		int C_YZ  = 5000;
		std::string SQLUpdateCols = "PRAGMA synchronous = OFF;"
		"PRAGMA journal_mode = OFF;"
		"BEGIN EXCLUSIVE TRANSACTION;"
		"PRAGMA defer_foreign_keys = ON;";
		for (auto &collection_fixed : kindex_list_fixed) {
			// UD_LOGI("Collection Fixed: %d - %s", collection_fixed["id"].get<int>(), collection_fixed["name"].get<std::string>().c_str());
			auto ColName_full1			=	collection_fixed["name"].get<std::string>();
			char ColName_FL 			=	ColName_full1[0];
			std::string Cold_ID			=	to_string(collection_fixed["id"]);
			// UD_LOGI("Collection Fixed: %s - %s", ColName_full.c_str(), ColName_FL.c_str());
			//ABC=1 / DEF=2 / GHI=3 / JKL=4 / MNO=5 / PQR=6 / STU=7 / VWX=8 / YZ=9;"
			if (Cold_ID == "256"){
					SQLUpdateCols.append("UPDATE collection_uniques SET name='Yamhan' WHERE id="+Cold_ID+";");
			}
			if (Lang == "fr") {
				if (Cold_ID == "431"){
					SQLUpdateCols.append("UPDATE collection_uniques SET name='C-8' WHERE id="+Cold_ID+";");
				}
			}
			if (ColName_FL == 'a' || ColName_FL == 'b' || ColName_FL == 'c') {
				SQLUpdateCols.append("UPDATE collection_uniques SET collection_category_id=1, priority="+std::to_string(C_ABC)+" WHERE id="+Cold_ID+";");
				C_ABC = C_ABC - 1;
				continue;
			} else if (ColName_FL == 'd' || ColName_FL == 'e' || ColName_FL == 'f') {
				SQLUpdateCols.append("UPDATE collection_uniques SET collection_category_id=2, priority="+std::to_string(C_DEF)+" WHERE id="+Cold_ID+";");
				C_DEF = C_DEF - 1;
				continue;
			} else if (ColName_FL == 'g' || ColName_FL == 'h' || ColName_FL == 'i') {
				SQLUpdateCols.append("UPDATE collection_uniques SET collection_category_id=3, priority="+std::to_string(C_GHI)+" WHERE id="+Cold_ID+";");
				C_GHI = C_GHI - 1;
				continue;
			} else if (ColName_FL == 'j' || ColName_FL == 'k' || ColName_FL == 'l') {
				SQLUpdateCols.append("UPDATE collection_uniques SET collection_category_id=4, priority="+std::to_string(C_JKL)+" WHERE id="+Cold_ID+";");
				C_JKL = C_JKL - 1;
				continue;
			} else if (ColName_FL == 'm' || ColName_FL == 'n' || ColName_FL == 'o') {
				SQLUpdateCols.append("UPDATE collection_uniques SET collection_category_id=5, priority="+std::to_string(C_MNO)+" WHERE id="+Cold_ID+";");
				C_MNO = C_MNO - 1;
				continue;
			} else if (ColName_FL == 'p' || ColName_FL == 'q' || ColName_FL == 'r') {
				SQLUpdateCols.append("UPDATE collection_uniques SET collection_category_id=6, priority="+std::to_string(C_PQR)+" WHERE id="+Cold_ID+";");
				C_PQR = C_PQR - 1;
				continue;
			} else if (ColName_FL == 's' || ColName_FL == 't' || ColName_FL == 'u') {
				SQLUpdateCols.append("UPDATE collection_uniques SET collection_category_id=7, priority="+std::to_string(C_STU)+" WHERE id="+Cold_ID+";");
				C_STU = C_STU - 1;
				continue;
			} else if (ColName_FL == 'v' || ColName_FL == 'w' || ColName_FL == 'x') {
				SQLUpdateCols.append("UPDATE collection_uniques SET collection_category_id=8, priority="+std::to_string(C_VWX)+" WHERE id="+Cold_ID+";");
				C_VWX = C_VWX - 1;
				continue;
			} else if (ColName_FL == 'y' || ColName_FL == 'z') {
				SQLUpdateCols.append("UPDATE collection_uniques SET collection_category_id=9, priority="+std::to_string(C_YZ)+" WHERE id="+Cold_ID+";");
				C_YZ = C_YZ - 1;
				continue;
			} else {
				//Should never be there
				UD_LOGI("IndexGLB : Issues L:225");
				UD_LOGI("Collection Fixed: FL : %s",ColName_FL);
				continue;
			}
		}
		SQLUpdateCols.append("COMMIT TRANSACTION;"
		"PRAGMA optimize;"
		"PRAGMA synchronous = NORMAL;"
		"PRAGMA journal_mode = DELETE;"
		"PRAGMA shrink_memory;");
		UD_LOGI("Collection Fixed: Begin update statements :");
		UniDokkan::Database::exec(SQLUpdateCols);
		UD_LOGI("Collection Fixed: END update statements");
		
	} else {
		UD_LOGI("IndexGLB : not FR or ENG language");
	}
}
void(*ori__CharacterMenuScene_onEnter)(void *self)= nullptr;
void CharacterMenuScene_onEnter(void *self) {
	UD_LOGI("IndexGLB : Init");
    ori__CharacterMenuScene_onEnter(self);
	auto screen_size = cocos2d_Director_getVisibleSize(UniDokkan::Dokkan::cocos2d_Director_getInstance());
	//Home Menu button
	//Check language :
	auto pathmp 		=	"layout/image/common/btn/menu_title_addon.png"; //Detect if music player is there
	auto file_utils		=	UniDokkan::Dokkan::cocos2d_FileUtils_getInstance();
	auto label_en		=	"layout/en/image/common/btn/menu_title_zukan.png";
	auto label_fr		=	"layout/fr/image/common/btn/menu_title_zukan.png";
	auto label_ja		=	"layout/ja/image/common/btn/menu_title_zukan.png";
	auto label_ko		=	"layout/ko/image/common/btn/menu_title_zukan.png";
	auto label_zh		=	"layout/zh/image/common/btn/menu_title_zukan.png";
	auto label_to_use	=	"layout/en/image/common/btn/menu_title_zukan.png";
	auto icon_to_use	=	"layout/en/image/common/btn/menu_icon_zukan.png";
	auto btn_to_use		=	"layout/en/image/common/btn/com_btn_menu_chara_gray.png";
	auto OriW			=	screen_size.width;
	auto OriH			=	screen_size.height;
	float Ypos			=	OriH/2;
	float YposN			=	Ypos;
	float XPos			=	OriW/2;
	float XPosN			=	XPos;
	auto Lang 			=	DetectLanguage();
	if (Lang == "en") {
		label_to_use	=	"layout/en/image/common/btn/menu_title_zukan.png";
		icon_to_use		=	"layout/en/image/common/btn/menu_icon_zukan.png";
		btn_to_use		=	"layout/en/image/common/btn/com_btn_menu_chara_gray.png";
		UD_LOGI("IndexGLB : EN found");
	} else if (Lang == "fr") {
		label_to_use	=	"layout/fr/image/common/btn/menu_title_zukan.png";
		icon_to_use		=	"layout/fr/image/common/btn/menu_icon_zukan.png";
		btn_to_use		=	"layout/fr/image/common/btn/com_btn_menu_chara_gray.png";
		UD_LOGI("IndexGLB : FR found");
	} else if (Lang == "zh") {
		label_to_use	=	"layout/zh/image/common/btn/menu_title_zukan.png";
		icon_to_use		=	"layout/zh/image/common/btn/menu_icon_zukan.png";
		btn_to_use		=	"layout/zh/image/common/btn/com_btn_menu_chara_gray.png";
		UD_LOGI("IndexGLB : zh found");
	} else if (Lang == "ko") {
		label_to_use	=	"layout/ko/image/common/btn/menu_title_zukan.png";
		icon_to_use		=	"layout/ko/image/common/btn/menu_icon_zukan.png";
		btn_to_use		=	"layout/ko/image/common/btn/com_btn_menu_chara_gray.png";
		UD_LOGI("IndexGLB : KO found");
	} else if (Lang == "ja") {
		label_to_use	=	"layout/ja/image/common/btn/menu_title_zukan.png";
		icon_to_use		=	"layout/ja/image/common/btn/menu_icon_zukan.png";
		btn_to_use		=	"layout/ja/image/common/btn/com_btn_menu_chara_gray.png";
		UD_LOGI("IndexGLB : JA found");
	} 
	if (UniDokkan::Dokkan::cocos2d_FileUtils_isFileExistWithoutCpk(file_utils, pathmp)) {
		YposN = Ypos-292.0f;
		XPosN = Ypos-292.0f;
	} else {
		YposN = Ypos-169.0f;
		XPosN = XPos;
	}
	std::string checkifaddedequip = "SELECT value FROM client_settings WHERE key='special_sticker_enabled';";
	auto data01 = UniDokkan::Database::select(checkifaddedequip);
	std::string curr_setting	=	to_string(data01[0]);
	float Xpos_1	=	XPosN;
	float Xpos_2	=	XPosN;
	float Xpos_3	=	XPosN;
	float Ypos_1	=	YposN;
	float Ypos_2	=	YposN;
	float Ypos_3	=	YposN;
	if (curr_setting == "{\"value\":\"0\"}") {
		Xpos_1	=	XPosN+162.0f;
		Xpos_2	=	XPosN+54.0f;
		Xpos_3	=	XPosN+194.0f;
		Ypos_1	=	YposN;
		Ypos_2	=	YposN+4.0f;
		Ypos_3	=	YposN;
	} else {
		Xpos_1	=	XPosN-151.0f; //-270
		Xpos_2	=	XPosN-249.0f;
		Xpos_3	=	XPosN-112.0f;
		
		Ypos_1	=	YposN-120.0f; // add 81 + marg
		Ypos_2	=	YposN-116.0f;
		Ypos_3	=	YposN-120.0f;
	}
	auto Open_index = new Button("open_indexbtn", self, 100);
    Open_index->setDefaultImage(btn_to_use);
    Open_index->setGravity(cocos2d::ui::LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL);
	Open_index->setPosition({Xpos_1, Ypos_1});

    auto left_icon = new ImageView("menu_icon_zukan", self, 200);
    left_icon->loadImage(icon_to_use);
    left_icon->setAnchorPoint(Node::AnchorCenter);;
    left_icon->setPosition({Xpos_2, Ypos_2});
	
    auto btn_label = new ImageView("menu_title_zukan", self, 200);
    btn_label->loadImage(label_to_use);
    btn_label->setAnchorPoint(Node::AnchorCenter);
    btn_label->setPosition({Xpos_3, Ypos_3});
	
    Open_index->addClickEventListener([=](Widget*) {
        auto collection_scene = DokkanFunction<void *()>("_ZN15CreateSceneUtil6createI22CollectionCardTopSceneJEEEPT_DpOT0_")();
        TransitionFade transition_fade(0.5f, cocos2d::Color3B::BLACK);
        Scene::pushScene(transition_fade.getRef(collection_scene));
    });
}

extern "C" {
    __attribute__ ((visibility ("default"))) 
    int unidokkan_init_v3(HookLibV3 *hook_lib) {
	UD_LOGI("IndexGLB custom module loading...");

	if (hook_lib->size < sizeof(HookLibV3)) {
		return UD_MODULE_INVALID_SIZE;
	}

	if (hook_lib->version < kMinPatcherVer) {
		return UD_MODULE_INVALID_VERSION;
	}

    UniDokkan::Database::addDatabaseReadyCallback(DatabaseReadyCallback);
    auto res = hook_lib->applyHooks({
			{DOKKAN_LIB, "_ZN18CharacterMenuScene7onEnterEv", reinterpret_cast<void *>(CharacterMenuScene_onEnter), reinterpret_cast<void **>(&ori__CharacterMenuScene_onEnter)},
    });
    if (!res) {
        return 1;
    }
        UD_LOGI("IndexGLB module successfully loaded.");
        return UD_MODULE_SUCCESS;
    }
}



extern "C" {
int unidokkan_init_v2(HookLib *hook_lib) {

    return 0;
}

}
