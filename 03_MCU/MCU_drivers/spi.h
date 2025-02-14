//=====================================================================
//閺傚洣娆㈤崥宥囆為敍姝磒i.h
//閸旂喕鍏樺鍌濐洣閿涙pi鎼存洖鐪版す鍗炲З閺嬪嫪娆㈠┃鎰瀮娴?
//閸掓湹缍旈崡鏇氱秴閿涙俺瀚冨鐐层亣鐎涳箑绁甸崗銉ョ础缁崵绮烘稉搴ｅ⒖閼辨梻缍夐惍鏃傗敀閹碉拷(sumcu.suda.edu.cn)
//閻?   閺堫剨绱? 2019-05-09  V2.0
//闁倻鏁ら懞顖滃閿涙L25閵嗕甫L26閵嗕甫L36
//=====================================================================
#ifndef _SPI_H             //闂冨弶顒涢柌宥咁槻鐎规矮绠熼敍?瀵拷婢?
#define _SPI_H

#include "string.h"

#define SPI_1  0
#define SPI_2  1
#define SPI_3  2

#define SPI_MASTER  1
#define SPI_SLAVE  0
//=====================================================================
//閸戣姤鏆熼崥宥囆為敍姝磒i_init閵?
//閸旂喕鍏樼拠瀛樻閿涙瓔PI閸掓繂顫愰崠?
//閸戣姤鏆熼崣鍌涙殶閿涙瓊o閿涙碍膩閸ф褰块敍灞藉讲閻劌寮弫鏉垮讲閸欏倽顫唃ec.h閺傚洣娆?
//       MSTR閿涙瓔PI娑撹绮犻張娲拷澶嬪閿?闁瀚ㄦ稉杞扮矤閺?1闁瀚ㄦ稉杞板瘜閺堟亽锟?
//       BaudRate閿涙碍灏濋悧鍦芳閿涘苯褰查崣?2000閵?000閵?000閵?000閵?500閵?000閿涘苯宕熸担宥忕窗b/s
//       CPOL閿涙POL=0閿涙岸鐝張澶嬫櫏SPI閺冨爼鎸撻敍鍫滅秵閺冪姵鏅ラ敍澶涚幢CPOL=1閿涙矮缍嗛張澶嬫櫏SPI閺冨爼鎸撻敍鍫ョ彯閺冪姵鏅ラ敍?
//       CPHA閿涙PHA=0閻╅晲缍呮稉?閿?CPHA=1閻╅晲缍呮稉?閿?
//閸戣姤鏆熸潻鏂挎礀閿涙碍妫?
//閸戣姤鏆熸径鍥ㄦ暈閿?CPHA=0閿涘本妞傞柦鐔朵繆閸欓娈戠粭顑跨娑擃亣绔熷▽鍨毉閻滄澘婀?閸涖劍婀￠弫鐗堝祦娴肩姾绶惃鍕儑娑擄拷娑擃亜鎳嗛張鐔烘畱娑擃厼銇庨敍?
//        CPHA=1閿涘本妞傞柦鐔朵繆閸欓娈戠粭顑跨娑擃亣绔熷▽鍨毉閻滄澘婀?閸涖劍婀￠弫鐗堝祦娴肩姾绶惃鍕儑娑擄拷娑擃亜鎳嗛張鐔烘畱鐠ч鍋ｉ妴?
//        CPHA=0閺冭绱濋柅姘繆閺堬拷缁嬪啿鐣鹃敍灞藉祮閹恒儲鏁归弬鐟版躬1/2閸涖劍婀￠惃鍕閸掕绮犵痪澶哥瑐閸欐牗鏆熼妴?
//=====================================================================
void spi_init(uint8_t No,uint8_t MSTR,uint16_t BaudRate,uint8_t CPOL,uint8_t CPHA);

//=====================================================================
//閸戣姤鏆熼崥宥囆為敍姝磒i_send1.
//閸旂喕鍏樼拠瀛樻閿涙瓔PI閸欐垿锟戒椒绔寸€涙濡弫鐗堝祦閵?
//閸戣姤鏆熼崣鍌涙殶閿涙瓊o閿涙碍膩閸ф褰块妴?閸欘垳鏁ら崣鍌涙殶閸欘垰寮憴涔琫c.h閺傚洣娆?
//       data閿?    闂囷拷鐟曚礁褰傞柅浣烘畱娑擄拷鐎涙濡弫鐗堝祦閵?
//閸戣姤鏆熸潻鏂挎礀閿?閿涙艾褰傞柅浣搞亼鐠愩儻绱?閿涙艾褰傞柅浣瑰灇閸旂喆锟?
//=====================================================================
uint8_t spi_send1(uint8_t No,uint8_t data);

//=====================================================================
//閸戣姤鏆熼崥宥囆為敍姝磒i_sendN.
//閸旂喕鍏樼拠瀛樻閿涙瓔PI閸欐垿锟戒焦鏆熼幑顔猴拷?
//閸戣姤鏆熼崣鍌涙殶閿涙瓊o閿涙碍膩閸ф褰块妴?閸忚泛褰囬崐闂磋礋0閹?
//       n:     鐟曚礁褰傞柅浣烘畱鐎涙濡稉顏呮殶閵嗗倽瀵栭崶缈犺礋(1~255)
//       data[]:閹碉拷閸欐垶鏆熺紒鍕畱妫ｆ牕婀撮崸锟介妴?
//閸戣姤鏆熸潻鏂挎礀閿涙碍妫ら妴?
//=====================================================================
uint8_t spi_sendN(uint8_t No,uint8_t n,uint8_t data[]);

//=====================================================================
//閸戣姤鏆熼崥宥囆為敍姝磒i_receive1.
//閸旂喕鍏樼拠瀛樻閿涙瓔PI閹恒儲鏁规稉锟芥稉顏勭摟閼哄倻娈戦弫鐗堝祦
//閸戣姤鏆熼崣鍌涙殶閿涙瓊o閿涙碍膩閸ф褰块敍灞藉讲閻劌寮弫鏉垮讲閸欏倽顫唃ec.h閺傚洣娆?
//閸戣姤鏆熸潻鏂挎礀閿涙碍甯撮弨璺哄煂閻ㄥ嫭鏆熼幑顔猴拷?
//=====================================================================
uint8_t spi_receive1(uint8_t No);

//=====================================================================
//閸戣姤鏆熼崥宥囆為敍姝磒i_receiveN.
//閸旂喕鍏樼拠瀛樻閿涙瓔PI閹恒儲鏁归弫鐗堝祦閵嗗倸缍媙=1閺冭绱濈亸杈ㄦЦ閹恒儱褰堟稉锟芥稉顏勭摟閼哄倻娈戦弫鐗堝祦閳ワ腹锟?
//閸戣姤鏆熼崣鍌涙殶閿涙瓊o閿涙碍膩閸ф褰块敍灞藉讲閻劌寮弫鏉垮讲閸欏倽顫唃ec.h閺傚洣娆?
//          n:    鐟曚礁褰傞柅浣烘畱鐎涙濡稉顏呮殶閵嗗倽瀵栭崶缈犺礋(1~255),
//       data[]:閹恒儲鏁归崚鎵畱閺佺増宓佺€涙ɑ鏂侀惃鍕浕閸︽澘娼冮妴?
//閸戣姤鏆熸潻鏂挎礀閿?閿涙碍甯撮弨鑸靛灇閸?閸忔湹绮幆鍛枌閿涙艾銇戠拹銉ｏ拷?
//=====================================================================
uint8_t spi_receiveN(uint8_t No,uint8_t n,uint8_t data[]);

//=====================================================================
//閸戣姤鏆熼崥宥囆為敍姝磒i_enable_re_int
//閸旂喕鍏樼拠瀛樻閿涙碍澧﹀锟絊PI閹恒儲鏁规稉顓熸焽閵?
//閸戣姤鏆熼崣鍌涙殶閿涙瓊o閿涙碍膩閸ф褰块敍灞藉讲閻劌寮弫鏉垮讲閸欏倽顫唃ec.h閺傚洣娆?
//閸戣姤鏆熸潻鏂挎礀閿涙碍妫ら妴?
//=====================================================================
void spi_enable_re_int(uint8_t No);

//=====================================================================
//閸戣姤鏆熼崥宥囆為敍姝磒i_disable_re_int
//閸旂喕鍏樼拠瀛樻閿涙艾鍙ч梻鐠朠I閹恒儲鏁规稉顓熸焽閵?
//閸戣姤鏆熼崣鍌涙殶閿涙瓊o閿涙碍膩閸ф褰块敍灞藉讲閻劌寮弫鏉垮讲閸欏倽顫唃ec.h閺傚洣娆?
//閸戣姤鏆熸潻鏂挎礀閿涙碍妫ら妴?
//=====================================================================
void spi_disable_re_int(uint8_t No);

#endif   //闂冨弶顒涢柌宥咁槻鐎规矮绠熼敍?缂佹挸鐔?

//=====================================================================
//婢圭増妲戦敍?
//閿?閿涘鍨滄禒顒€绱戦崣鎴犳畱濠ф劒鍞惍渚婄礉閸︺劍婀版稉顓炵妇閹绘劒绶甸惃鍕€栨禒鍓侀兇缂佺喐绁寸拠鏇拷姘崇箖閿涘瞼婀＄拠姘殠閻氼喚绮扮粈鍙ョ窗閿涘奔绗夌搾鍏呯婢跺嫸绱?
//     濞嗐垼绻嬮幐鍥劀閵?
//閿?閿涘顕禍搴濆▏閻劑娼張顑胯厬韫囧啰鈥栨禒鍓侀兇缂佺喓娈戦悽銊﹀煕閿涘瞼些濡炲秳鍞惍浣规閿涘矁顕禒鏃傜矎閺嶈宓侀懛顏勭箒閻ㄥ嫮鈥栨禒璺哄爱闁板秲锟?
//
//閼诲繐绐炴径褍顒烴XP瀹撳苯鍙嗗蹇庤厬韫?http://sumcu.suda.edu.cn,0512-65214835)
