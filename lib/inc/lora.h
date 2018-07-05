/*! \file lora.h
    \brief  LoRa Module에 대한 설정, 전송 및 수신 함수가 정의된 헤더파일입니다.
 */
/*! \fn void InitLora(Configuration *)
    \brief LoRa LoRa 전송 및 수신에 필요한 파라미터 정보를 구조체로부터 받아 LoRa Module을 초기화하는 함수입니다.
    \param config_data 실험정보(LoRa 파라미터)가 담겨있는 구조체
 */
 /*!
    \fn int SendLora(MyPacket *)
    \brief Lora Module을 통해 단일 패킷 전송합니다.
    \param packet gateway에 보내고자하는패킷
    \return snd_state 전송 성공시 0, 실행중 오류발생시 1, 미동작시 2 반환
 */
 /*!
    \fn MyPacket ReceiveLora(int)
    \brief wait_time 동안 LoRa Module을 통해 단일 패킷을 수신합니다.
    \param wait_time 패킷에 대한 최대 수신 대기시간
    \return rcv_packet 수신 성공시 수신한 패킷, wait 시간 안에 충돌 혹은 수신 실패시, 가상의 패킷 반환
*/

#include "arduPiLoRa.h"
#include <time.h>
#include <cmath>

void init_lora(uint8_t p_mode, uint32_t p_channel, char p_power, uint8_t p_srcAddr, uint8_t p_dstAddr);
