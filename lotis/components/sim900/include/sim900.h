#ifndef SIM900_H
#define SIM900_H
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"


#ifdef __cplusplus
extern "C" {
#endif
/* ========================================================================== /
/                             CONFIGURATION                                   /
/ ========================================================================== */
/**

@brief SIM900A Module Configuration Structure
*/
typedef struct {
uint32_t baud_rate;              /* UART baud rate (default: 115200) */
uart_port_t uart_port;           /* UART port number */
int rx_pin;                      /* UART RX GPIO pin */
int tx_pin;                      /* UART TX GPIO pin */
uart_word_length_t data_bits;    /* UART data bits */
uart_parity_t parity;            /* UART parity */
uart_stop_bits_t stop_bits;      /* UART stop bits */
uart_hw_flowcontrol_t flow_ctrl; /* Hardware flow control */
uint8_t rx_flow_ctrl_thresh;     /* RX flow control threshold */
} sim900_config_t;

/* ========================================================================== /
/                        AT COMMAND DEFINITIONS                              /
/ ========================================================================== */
/* ----------------------------- Basic Commands ----------------------------- */
/**

@brief Re-issues the last command given
@usage Send this to repeat the previous AT command
*/
#define AT_REPEAT_LAST "A/\r"

/**

@brief Answer an incoming call
@usage Send when RING URC is received to accept voice call
*/
#define AT_ANSWER "ATA\r"

/**

@brief Dial a number for voice call
@param number Phone number to dial (include ; for voice)
@usage ATD13800138000; for voice call
*/
#define AT_DIAL "ATD"

/**

@brief Set command echo mode
@param n 0=off, 1=on
@usage ATE0 to disable echo, ATE1 to enable
*/
#define AT_ECHO_OFF "ATE0\r"
#define AT_ECHO_ON "ATE1\r"

/**

@brief Hang up call
@usage Send to terminate active call
*/
#define AT_HANGUP "ATH\r"

/**

@brief Display product identification
@usage Returns module model and revision
*/
#define AT_GET_INFO "ATI\r"

/**

@brief Switch from data mode to command mode
@usage Send after 1s pause, wait 0.5s after
*/
#define AT_ESCAPE_SEQ "+++"

/**

@brief Switch from command mode to data mode
@usage Resume data connection after +++
*/
#define AT_ONLINE "ATO\r"

/**

@brief Set number of rings before auto-answer
@param n 0=disable, 1-255=ring count
@usage ATS0=2 to answer after 2 rings
*/
#define AT_SET_AUTOANSWER "ATS0="

/**

@brief Set TA response format
@param n 0=numeric, 1=verbose
@usage ATV1 for text responses (OK, ERROR)
*/
#define AT_VERBOSE_ON "ATV1\r"
#define AT_VERBOSE_OFF "ATV0\r"

/**

@brief Reset to default configuration
@usage Restore factory settings
*/
#define AT_RESET "ATZ\r"

/**

@brief Set DCD function mode
@param n 0=always ON, 1=ON when carrier present
*/
#define AT_SET_DCD "AT&C"

/**

@brief Set DTR function mode
@param n 0=ignore, 1=command mode, 2=disconnect
*/
#define AT_SET_DTR "AT&D"

/**

@brief Factory defined configuration
@usage Reset all settings to factory defaults
*/
#define AT_FACTORY_RESET "AT&F\r"

/**

@brief Display current configuration
@usage Show active profile parameters
*/
#define AT_DISPLAY_CONFIG "AT&V\r"

/**

@brief Store active profile to non-volatile memory
@usage Save current settings
*/
#define AT_SAVE_PROFILE "AT&W\r"

/**

@brief Request complete TA capabilities list
@usage Returns supported features (+CGSM, +FCLASS)
*/
#define AT_GET_CAPABILITIES "AT+GCAP\r"

/**

@brief Request manufacturer identification
@usage Returns "SIMCOM_Ltd"
*/
#define AT_GET_MANUFACTURER "AT+GMI\r"

/**

@brief Request model identification
@usage Returns model name (e.g., SIM900)
*/
#define AT_GET_MODEL "AT+GMM\r"

/**

@brief Request revision identification
@usage Returns firmware version
*/
#define AT_GET_REVISION "AT+GMR\r"

/**

@brief Request serial number (IMEI)
@usage Returns 15-digit IMEI
*/
#define AT_GET_IMEI "AT+GSN\r"

/**

@brief Set TE-TA character framing
@param format,parity Character format and parity
@usage AT+ICF=3,0 for 8N1
*/
#define AT_SET_FRAMING "AT+ICF="

/**

@brief Set TE-TA flow control
@param dce_by_dte,dte_by_dce Flow control methods
@usage AT+IFC=2,2 for RTS/CTS
*/
#define AT_SET_FLOW_CTRL "AT+IFC="

/**

@brief Set TE-TA fixed baud rate
@param rate Baud rate (0=auto, 1200-115200)
@usage AT+IPR=115200 for fixed rate
*/
#define AT_SET_BAUDRATE "AT+IPR="

/* --------------------------- GSM 07.07 Commands --------------------------- */
/**

@brief Accumulated call meter reset/query
@usage AT+CACM? to query, AT+CACM=PIN2 to reset
*/
#define AT_CALL_METER "AT+CACM"

/**

@brief Set/query call meter maximum
@param acmmax,passwd Maximum value and PIN2
@usage AT+CAMM="00001E",PIN2
*/
#define AT_CALL_METER_MAX "AT+CAMM"

/**

@brief Advice of charge
@param mode 0=query, 1=disable URC, 2=enable URC
@usage AT+CAOC=0 to query current call cost
*/
#define AT_ADVICE_CHARGE "AT+CAOC="

/**

@brief Select bearer service type
@param speed,name,ce Data rate and service
@usage AT+CBST=71,0,1 for 9600bps non-transparent
*/
#define AT_BEARER_SERVICE "AT+CBST="

/**

@brief Call forwarding control
@param reason,mode,number,type Forward conditions
@usage AT+CCFC=0,1 to enable unconditional forward
*/
#define AT_CALL_FORWARD "AT+CCFC="

/**

@brief Call waiting control
@param n,mode,class Enable/disable call waiting
@usage AT+CCWA=1,1 to enable for voice
*/
#define AT_CALL_WAITING "AT+CCWA="

/**

@brief Extended error report
@usage Returns detailed error for last call failure
*/
#define AT_EXTENDED_ERROR "AT+CEER\r"

/**

@brief Select TE character set
@param chset "GSM","IRA","UCS2","HEX","PCCP","8859-1"
@usage AT+CSCS="GSM" for default GSM alphabet
*/
#define AT_SET_CHARSET "AT+CSCS="

/**

@brief Select type of address
@param type 129=national, 145=international
@usage AT+CSTA=145 for international format
*/
#define AT_SET_ADDR_TYPE "AT+CSTA="

/**

@brief Call hold and multiparty
@param n Call hold operation
@usage AT+CHLD=0 release held calls
*/
#define AT_CALL_HOLD "AT+CHLD="

/**

@brief Request IMSI
@usage Returns 15-digit IMSI from SIM
*/
#define AT_GET_IMSI "AT+CIMI\r"

/**

@brief List current calls
@param n 0=disable auto report, 1=enable
@usage AT+CLCC to list all active calls
*/
#define AT_LIST_CALLS "AT+CLCC"

/**

@brief Facility lock
@param fac,mode,passwd Lock/unlock facilities
@usage AT+CLCK="SC",1,PIN to lock SIM
*/
#define AT_FACILITY_LOCK "AT+CLCK="

/**

@brief Calling line identification presentation
@param n 0=disable, 1=enable
@usage AT+CLIP=1 to show caller number
*/
#define AT_CLIP "AT+CLIP="

/**

@brief Calling line identification restriction
@param n 0=default, 1=restrict, 2=allow
@usage AT+CLIR=1 to hide own number
*/
#define AT_CLIR "AT+CLIR="

/**

@brief Report mobile equipment error
@param n 0=disable, 1=numeric, 2=verbose
@usage AT+CMEE=2 for detailed error messages
*/
#define AT_ERROR_REPORT "AT+CMEE="

/**

@brief Connected line identification
@param n 0=disable, 1=enable
@usage AT+COLP=1 to show connected number
*/
#define AT_COLP "AT+COLP="

/**

@brief Operator selection
@param mode,format,oper Select network operator
@usage AT+COPS=0 for automatic selection
*/
#define AT_OPERATOR_SELECT "AT+COPS="

/**

@brief Phone activity status
@usage Returns 0=ready, 3=ringing, 4=call in progress
*/
#define AT_PHONE_ACTIVITY "AT+CPAS\r"

/**

@brief Find phonebook entries
@param findtext Search string
@usage AT+CPBF="John" to find entries
*/
#define AT_PHONEBOOK_FIND "AT+CPBF="

/**

@brief Read phonebook entries
@param index1,index2 Range of entries
@usage AT+CPBR=1,10 to read first 10
*/
#define AT_PHONEBOOK_READ "AT+CPBR="

/**

@brief Select phonebook storage
@param storage "SM"=SIM, "ME"=phone, "ON"=own numbers
@usage AT+CPBS="SM" for SIM phonebook
*/
#define AT_PHONEBOOK_SELECT "AT+CPBS="

/**

@brief Write phonebook entry
@param index,number,type,text Entry details
@usage AT+CPBW=1,"13800138000",129,"Test"
*/
#define AT_PHONEBOOK_WRITE "AT+CPBW="

/**

@brief Enter PIN
@param pin,newpin PIN code and optional new PIN
@usage AT+CPIN="1234" to unlock SIM
*/
#define AT_ENTER_PIN "AT+CPIN="

/**

@brief Change password
@param fac,oldpwd,newpwd Facility and passwords
@usage AT+CPWD="SC","1234","5678"
*/
#define AT_CHANGE_PASSWORD "AT+CPWD="

/**

@brief Service reporting control
@param mode 0=disable, 1=enable
@usage AT+CR=1 to show service type
*/
#define AT_SERVICE_REPORT "AT+CR="

/**

@brief Cellular result codes
@param mode 0=disable, 1=enable extended RING
@usage AT+CRC=1 for +CRING: indication
*/
#define AT_CELLULAR_RESULT "AT+CRC="

/**

@brief Network registration
@param n 0=disable, 1=enable, 2=enable with location
@usage AT+CREG=1 to enable registration URC
*/
#define AT_NETWORK_REG "AT+CREG="

/**

@brief Restricted SIM access
@param command,fileid,p1,p2,p3,data SIM file access
@usage AT+CRSM=176,28539,0,0,10 to read file
*/
#define AT_SIM_ACCESS "AT+CRSM="

/**

@brief Signal quality report
@usage Returns RSSI (0-31) and BER (0-7,99)
*/
#define AT_SIGNAL_QUALITY "AT+CSQ\r"

/**

@brief DTMF tone generation
@param dtmf-string Tones to play
@usage AT+VTS="1234" to play DTMF tones
*/
#define AT_DTMF_TONE "AT+VTS="

/**

@brief Multiplexer control
@param mode,subset,port_speed,N1,T1,N2,T2,T3,k MUX params
@usage AT+CMUX=0 for basic multiplexing
*/
#define AT_MULTIPLEX "AT+CMUX="

/**

@brief Subscriber number
@usage Returns MSISDN from SIM
*/
#define AT_SUBSCRIBER_NUM "AT+CNUM\r"

/**

@brief Preferred operator list
@param index,format,oper Preferred PLMN
@usage AT+CPOL=1,2,"46000" to set preference
*/
#define AT_PREFERRED_PLMN "AT+CPOL="

/**

@brief Read operator names
@usage Returns list of operator names
*/
#define AT_OPERATOR_NAMES "AT+COPN\r"

/**

@brief Set phone functionality
@param fun,rst 0=minimum, 1=full, 4=flight mode
@usage AT+CFUN=1 for full functionality
*/
#define AT_PHONE_FUNC "AT+CFUN="

/**

@brief Clock management
@param time Date/time string
@usage AT+CCLK="21/01/01,12:00:00+32"
*/
#define AT_SET_CLOCK "AT+CCLK="

/**

@brief Generic SIM access
@param length,command SIM command in hex
@usage AT+CSIM=14,"A0A40000027F10"
*/
#define AT_GENERIC_SIM "AT+CSIM="

/**

@brief Alert sound mode
@param mode 0=normal, 1=silent
@usage AT+CALM=0 to enable sounds
*/
#define AT_ALERT_MODE "AT+CALM="

/**

@brief Alert sound select
@param n 0-19 Sound type
@usage AT+CALS=5 to select alert type 5
*/
#define AT_ALERT_SELECT "AT+CALS="

/**

@brief Ringer sound level
@param level 0=off, 1=low, 2=med, 3=high, 4=crescendo
@usage AT+CRSL=3 for high volume
*/
#define AT_RINGER_LEVEL "AT+CRSL="

/**

@brief Speaker volume level
@param level 0-100
@usage AT+CLVL=50 for 50% volume
*/
#define AT_SPEAKER_VOLUME "AT+CLVL="

/**

@brief Mute control
@param n 0=unmute, 1=mute
@usage AT+CMUT=1 to mute microphone
*/
#define AT_MUTE "AT+CMUT="

/**

@brief Price per unit and currency
@param currency,ppu Currency code and price
@usage AT+CPUC="USD","0.01",PIN2
*/
#define AT_PRICE_CURRENCY "AT+CPUC="

/**

@brief Call meter maximum event
@param mode 0=disable, 1=enable warning
@usage AT+CCWE=1 to enable ACM warning
*/
#define AT_CALL_METER_WARN "AT+CCWE="

/**

@brief Battery charge status
@usage Returns charge status and level
*/
#define AT_BATTERY_CHARGE "AT+CBC\r"

/**

@brief USSD commands
@param n,str,dcs USSD code and encoding
@usage AT+CUSD=1,"*123#" to send USSD
*/
#define AT_USSD "AT+CUSD="

/**

@brief Supplementary service notification
@param n,m Enable/disable SS notifications
@usage AT+CSSN=1,1 to enable all notifications
*/
#define AT_SS_NOTIFICATION "AT+CSSN="

/* --------------------------- GSM 07.05 SMS Commands ----------------------- */
/**

@brief Delete SMS message
@param index,delflag Message index and delete flag
@usage AT+CMGD=1 to delete message 1
*/
#define AT_SMS_DELETE "AT+CMGD="

/**

@brief Select SMS message format
@param mode 0=PDU, 1=text
@usage AT+CMGF=1 for text mode (easier)
*/
#define AT_SMS_FORMAT "AT+CMGF="

/**

@brief List SMS messages
@param stat "ALL","REC UNREAD","REC READ","STO UNSENT","STO SENT"
@usage AT+CMGL="ALL" to list all messages
*/
#define AT_SMS_LIST "AT+CMGL="

/**

@brief Read SMS message
@param index,mode Message index
@usage AT+CMGR=1 to read message at index 1
*/
#define AT_SMS_READ "AT+CMGR="

/**

@brief Send SMS message
@param da,toda Destination address
@usage AT+CMGS="13800138000" then type message and Ctrl-Z
*/
#define AT_SMS_SEND "AT+CMGS="

/**

@brief Write SMS to memory
@param oa/da,tooa/toda Address for stored message
@usage AT+CMGW="13800138000" to store message
*/
#define AT_SMS_WRITE "AT+CMGW="

/**

@brief Send SMS from storage
@param index,da,toda Send stored message
@usage AT+CMSS=1 to send message at index 1
*/
#define AT_SMS_SEND_STORED "AT+CMSS="

/**

@brief New SMS indication
@param mode,mt,bm,ds,bfr Indication settings
@usage AT+CNMI=2,1,0,0,0 for direct routing
*/
#define AT_SMS_NEW_IND "AT+CNMI="

/**

@brief Preferred SMS storage
@param mem1,mem2,mem3 Read/Write/Receive storage
@usage AT+CPMS="SM","SM","SM" for SIM storage
*/
#define AT_SMS_STORAGE "AT+CPMS="

/**

@brief Restore SMS settings
@param profile Profile number (0-1)
@usage AT+CRES=0 to restore profile 0
*/
#define AT_SMS_RESTORE "AT+CRES="

/**

@brief Save SMS settings
@param profile Profile number (0-1)
@usage AT+CSAS=0 to save to profile 0
*/
#define AT_SMS_SAVE "AT+CSAS="

/**

@brief SMS service center address
@param sca,tosca SMSC number and type
@usage AT+CSCA="+8613800138000",145
*/
#define AT_SMS_CENTER "AT+CSCA="

/**

@brief Cell broadcast SMS selection
@param mode,mids,dcss Enable/disable CB messages
@usage AT+CSCB=0,"0,1,5",,"0,1" to accept types
*/
#define AT_SMS_CELL_BROADCAST "AT+CSCB="

/**

@brief Show SMS text mode parameters
@param show 0=hide, 1=show headers
@usage AT+CSDH=1 to show full SMS details
*/
#define AT_SMS_SHOW_PARAMS "AT+CSDH="

/**

@brief Set SMS text mode parameters
@param fo,vp,pid,dcs SMS parameters
@usage AT+CSMP=17,167,0,0 for default
*/
#define AT_SMS_SET_PARAMS "AT+CSMP="

/**

@brief Select message service
@param service 0=GSM 03.40, 1=GSM 03.41
@usage AT+CSMS=0 for standard SMS
*/
#define AT_SMS_SERVICE "AT+CSMS="

/* -------------------------- GPRS Commands --------------------------------- */
/**

@brief GPRS attach/detach
@param state 0=detach, 1=attach
@usage AT+CGATT=1 to attach to GPRS
*/
#define AT_GPRS_ATTACH "AT+CGATT="

/**

@brief Define PDP context
@param cid,PDP_type,APN,PDP_addr Context definition
@usage AT+CGDCONT=1,"IP","internet"
*/
#define AT_PDP_CONTEXT "AT+CGDCONT="

/**

@brief QoS profile minimum acceptable
@param cid,precedence,delay,reliability,peak,mean QoS params
@usage AT+CGQMIN=1,0,0,0,0,0
*/
#define AT_QOS_MIN "AT+CGQMIN="

/**

@brief QoS profile requested
@param cid,precedence,delay,reliability,peak,mean QoS params
@usage AT+CGQREQ=1,0,0,0,0,0
*/
#define AT_QOS_REQ "AT+CGQREQ="

/**

@brief PDP context activate/deactivate
@param state,cid Activation state
@usage AT+CGACT=1,1 to activate context 1
*/
#define AT_PDP_ACTIVATE "AT+CGACT="

/**

@brief Enter data state
@param L2P,cid Layer 2 protocol
@usage AT+CGDATA="PPP",1 for PPP connection
*/
#define AT_ENTER_DATA "AT+CGDATA="

/**

@brief Show PDP address
@param cid Context ID
@usage AT+CGPADDR=1 to get IP address
*/
#define AT_PDP_ADDRESS "AT+CGPADDR="

/**

@brief GPRS mobile station class
@param class "B"=Class B, "CC"=Class CC
@usage AT+CGCLASS="B" for packet+circuit switching
*/
#define AT_GPRS_CLASS "AT+CGCLASS="

/**

@brief Control GPRS event reporting
@param mode,bfr Event reporting mode
@usage AT+CGEREP=1,0 to enable events
*/
#define AT_GPRS_EVENT "AT+CGEREP="

/**

@brief GPRS network registration
@param n 0=disable, 1=enable, 2=enable with location
@usage AT+CGREG=1 to enable GPRS reg URC
*/
#define AT_GPRS_REG "AT+CGREG="

/**

@brief Select service for MO SMS
@param service 0=GPRS, 1=circuit, 2=GPRS preferred, 3=circuit preferred
@usage AT+CGSMS=2 for GPRS preferred
*/
#define AT_SMS_SERVICE_SELECT "AT+CGSMS="

/* ----------------------- TCP/IP Application Commands --------------------- */
/**

@brief Start multi-IP connection
@param n 0=single, 1=multi
@usage AT+CIPMUX=0 for single connection
*/
#define AT_IP_MULTI "AT+CIPMUX="

/**

@brief Start TCP/UDP connection
@param n,mode,address,port Connection parameters
@usage AT+CIPSTART="TCP","www.google.com",80
*/
#define AT_IP_START "AT+CIPSTART="

/**

@brief Send data through connection
@param length Data length
@usage AT+CIPSEND=100 then send 100 bytes
*/
#define AT_IP_SEND "AT+CIPSEND="

/**

@brief Select data transmitting mode
@param n 0=normal, 1=quick send
@usage AT+CIPQSEND=0 for acknowledgment
*/
#define AT_IP_QUICK_SEND "AT+CIPQSEND="

/**

@brief Query data transmitting state
@param n Connection number (multi-IP)
@usage AT+CIPACK to check sent/acked bytes
*/
#define AT_IP_ACK "AT+CIPACK"

/**

@brief Close connection
@param n,id Close mode and connection ID
@usage AT+CIPCLOSE=1 for quick close
*/
#define AT_IP_CLOSE "AT+CIPCLOSE="

/**

@brief Deactivate GPRS PDP context
@usage AT+CIPSHUT to close all connections
*/
#define AT_IP_SHUTDOWN "AT+CIPSHUT\r"

/**

@brief Set local port
@param mode,port Protocol and port number
@usage AT+CLPORT="TCP",8080
*/
#define AT_IP_LOCAL_PORT "AT+CLPORT="

/**

@brief Start task and set APN
@param apn,user,pwd APN credentials
@usage AT+CSTT="internet","","" for APN
*/
#define AT_IP_SET_APN "AT+CSTT="

/**

@brief Bring up wireless connection
@usage AT+CIICR to activate GPRS
*/
#define AT_IP_BRING_UP "AT+CIICR\r"

/**

@brief Get local IP address
@usage AT+CIFSR returns assigned IP
*/
#define AT_IP_GET_IP "AT+CIFSR\r"

/**

@brief Query connection status
@param n Connection number (multi-IP)
@usage AT+CIPSTATUS to check all connections
*/
#define AT_IP_STATUS "AT+CIPSTATUS"

/**

@brief Configure DNS server
@param pri_dns,sec_dns Primary and secondary DNS
@usage AT+CDNSCFG="8.8.8.8","8.8.4.4"
*/
#define AT_DNS_CONFIG "AT+CDNSCFG="

/**

@brief Query IP address of domain
@param domain Domain name
@usage AT+CDNSGIP="www.google.com"
*/
#define AT_DNS_QUERY "AT+CDNSGIP="

/**

@brief Add IP header to received data
@param mode 0=off, 1=on (+IPD,len:)
@usage AT+CIPHEAD=1 to show data length
*/
#define AT_IP_HEAD "AT+CIPHEAD="

/**

@brief Set auto sending timer
@param mode,time Enable timer and timeout
@usage AT+CIPATS=1,10 for 10s auto-send
*/
#define AT_IP_AUTO_SEND "AT+CIPATS="

/**

@brief Set prompt when sending
@param send_prompt 0=no >, 1=show >, 2=neither
@usage AT+CIPSPRT=1 to show > prompt
*/
#define AT_IP_SEND_PROMPT "AT+CIPSPRT="

/**

@brief Configure as server
@param mode,port 0=close, 1=open server
@usage AT+CIPSERVER=1,8080 to listen
*/
#define AT_IP_SERVER "AT+CIPSERVER="

/**

@brief Set CSD or GPRS mode
@param mode,apn/dial Connection type and params
@usage AT+CIPCSGP=1,"internet"
*/
#define AT_IP_CONNECTION_MODE "AT+CIPCSGP="

/**

@brief Show remote IP when receiving
@param mode 0=off, 1=show IP:port
@usage AT+CIPSRIP=1 to show sender info
*/
#define AT_IP_REMOTE_IP "AT+CIPSRIP="

/**

@brief Set PDP context check timing
@param mode,interval,timer
* @usage AT+CIPDPDP=1,60,3 check every 60s
 */
#define AT_IP_PDP_CHECK "AT+CIPDPDP="

/**
 * @brief Select TCP/IP application mode
 * @param mode 0=normal, 1=transparent
 * @usage AT+CIPMODE=0 for command mode
 */
#define AT_IP_MODE "AT+CIPMODE="

/**
 * @brief Configure transparent transfer mode
 * @param NmRetry,WaitTm,SendSz,esc Transfer parameters
 * @usage AT+CIPCCFG=3,2,1024,1
 */
#define AT_IP_TRANSPARENT_CFG "AT+CIPCCFG="

/**
 * @brief Display transfer protocol in header
 * @param mode 0=off, 1=show TCP/UDP
 * @usage AT+CIPSHOWTP=1 to show protocol
 */
#define AT_IP_SHOW_PROTOCOL "AT+CIPSHOWTP="

/**
 * @brief UDP extended mode
 * @param mode,IP,port 0=normal, 1=extended, 2=set address
 * @usage AT+CIPUDPMODE=1 for extended mode
 */
#define AT_UDP_MODE "AT+CIPUDPMODE="

/**
 * @brief Get data from network manually
 * @param mode,id,len 1=enable manual, 2=read data
 * @usage AT+CIPRXGET=2,1460 to read 1460 bytes
 */
#define AT_IP_RX_GET "AT+CIPRXGET="

/**
 * @brief Quick remote close
 * @param mode 0=FIN frame, 1=RST frame
 * @usage AT+CIPQRCLOSE=1 for fast close
 */
#define AT_IP_QUICK_CLOSE "AT+CIPQRCLOSE="

/**
 * @brief Save TCP/IP application context
 * @usage AT+CIPSCONT to save settings to NVRAM
 */
#define AT_IP_SAVE_CONTEXT "AT+CIPSCONT\r"

/* ----------------------- IP Application Commands -------------------------- */

/**
 * @brief Bearer settings for IP applications
 * @param cmd_type,cid,tag,value Bearer configuration
 * @usage AT+SAPBR=3,1,"APN","internet" to set APN
 *        AT+SAPBR=1,1 to open bearer
 *        AT+SAPBR=2,1 to query bearer status
 */
#define AT_BEARER_SETTINGS "AT+SAPBR="

/* ----------------------- HTTP Application Commands ------------------------ */

/**
 * @brief Initialize HTTP service
 * @usage AT+HTTPINIT before HTTP operations
 */
#define AT_HTTP_INIT "AT+HTTPINIT\r"

/**
 * @brief Terminate HTTP service
 * @usage AT+HTTPTERM after HTTP operations
 */
#define AT_HTTP_TERM "AT+HTTPTERM\r"

/**
 * @brief Set HTTP parameters
 * @param tag,value Parameter name and value
 * @usage AT+HTTPPARA="URL","http://www.example.com"
 *        AT+HTTPPARA="CID",1
 */
#define AT_HTTP_PARAM "AT+HTTPPARA="

/**
 * @brief Input HTTP data for POST
 * @param size,time Data size and input timeout
 * @usage AT+HTTPDATA=1024,10000 then input data
 */
#define AT_HTTP_DATA "AT+HTTPDATA="

/**
 * @brief HTTP method action
 * @param method 0=GET, 1=POST, 2=HEAD
 * @usage AT+HTTPACTION=0 for HTTP GET
 */
#define AT_HTTP_ACTION "AT+HTTPACTION="

/**
 * @brief Read HTTP server response
 * @param start_addr,byte_size Read range
 * @usage AT+HTTPREAD=0,1000 to read 1000 bytes
 */
#define AT_HTTP_READ "AT+HTTPREAD="

/**
 * @brief Save HTTP application context
 * @usage AT+HTTPSCONT to save HTTP settings
 */
#define AT_HTTP_SAVE_CONTEXT "AT+HTTPSCONT\r"

/* ----------------------- FTP Application Commands ------------------------- */

/**
 * @brief Set FTP control port
 * @param port Port number (default 21)
 * @usage AT+FTPPORT=21
 */
#define AT_FTP_PORT "AT+FTPPORT="

/**
 * @brief Set FTP mode
 * @param mode 0=active, 1=passive
 * @usage AT+FTPMODE=1 for passive (recommended)
 */
#define AT_FTP_MODE "AT+FTPMODE="

/**
 * @brief Set FTP transfer type
 * @param type "A"=ASCII, "I"=binary
 * @usage AT+FTPTYPE="I" for binary
 */
#define AT_FTP_TYPE "AT+FTPTYPE="

/**
 * @brief Set FTP put type
 * @param type "APPE","STOU","STOR"
 * @usage AT+FTPPUTOPT="STOR" for overwrite
 */
#define AT_FTP_PUT_OPT "AT+FTPPUTOPT="

/**
 * @brief Set FTP bearer profile ID
 * @param cid Bearer profile identifier
 * @usage AT+FTPCID=1
 */
#define AT_FTP_CID "AT+FTPCID="

/**
 * @brief Set resume broken download
 * @param value Byte position to resume from
 * @usage AT+FTPREST=1024 to resume from byte 1024
 */
#define AT_FTP_RESUME "AT+FTPREST="

/**
 * @brief Set FTP server address
 * @param server IP or domain name
 * @usage AT+FTPSERV="ftp.example.com"
 */
#define AT_FTP_SERVER "AT+FTPSERV="

/**
 * @brief Set FTP username
 * @param username FTP login username
 * @usage AT+FTPUN="myusername"
 */
#define AT_FTP_USERNAME "AT+FTPUN="

/**
 * @brief Set FTP password
 * @param password FTP login password
 * @usage AT+FTPPW="mypassword"
 */
#define AT_FTP_PASSWORD "AT+FTPPW="

/**
 * @brief Set download file name
 * @param filename File to download
 * @usage AT+FTPGETNAME="test.txt"
 */
#define AT_FTP_GET_NAME "AT+FTPGETNAME="

/**
 * @brief Set download file path
 * @param path Directory path
 * @usage AT+FTPGETPATH="/pub/data/"
 */
#define AT_FTP_GET_PATH "AT+FTPGETPATH="

/**
 * @brief Set upload file name
 * @param filename File to upload
 * @usage AT+FTPPUTNAME="upload.bin"
 */
#define AT_FTP_PUT_NAME "AT+FTPPUTNAME="

/**
 * @brief Set upload file path
 * @param path Directory path
 * @usage AT+FTPPUTPATH="/upload/"
 */
#define AT_FTP_PUT_PATH "AT+FTPPUTPATH="

/**
 * @brief Download file
 * @param mode 1=open session, 2=read data
 * @usage AT+FTPGET=1 then AT+FTPGET=2,1024
 */
#define AT_FTP_GET "AT+FTPGET="

/**
 * @brief Upload file
 * @param mode,reqlength 1=open session, 2=write data
 * @usage AT+FTPPUT=1 then AT+FTPPUT=2,1024
 */
#define AT_FTP_PUT "AT+FTPPUT="

/**
 * @brief Save FTP application context
 * @usage AT+FTPSCONT to save FTP settings
 */
#define AT_FTP_SAVE_CONTEXT "AT+FTPSCONT\r"

/* -------------------- SIMCom Specific Commands --------------------------- */

/**
 * @brief Change side tone gain level
 * @param channel,gainlevel Audio channel and gain
 * @usage AT+SIDET=0,5 for handset gain level 5
 */
#define AT_SIDE_TONE "AT+SIDET="

/**
 * @brief Power off module
 * @param n 0=urgent, 1=normal
 * @usage AT+CPOWD=1 for graceful shutdown
 */
#define AT_POWER_OFF "AT+CPOWD="

/**
 * @brief Times remained to input SIM PIN/PUK
 * @usage AT+SPIC to check remaining PIN attempts
 */
#define AT_PIN_REMAIN "AT+SPIC\r"

/**
 * @brief Change microphone gain level
 * @param channel,gainlevel Audio channel and gain (0-15)
 * @usage AT+CMIC=0,10 for handset mic gain
 */
#define AT_MIC_GAIN "AT+CMIC="

/**
 * @brief Set alarm time
 * @param time,n,recurr Alarm time and recurrence
 * @usage AT+CALA="21/12/25,08:00:00+32",1,"1,2,3,4,5"
 */
#define AT_ALARM_SET "AT+CALA="

/**
 * @brief Delete alarm
 * @param n Alarm index (1-5)
 * @usage AT+CALD=1 to delete alarm 1
 */
#define AT_ALARM_DELETE "AT+CALD="

/**
 * @brief Read ADC value
 * @usage AT+CADC? to read ADC voltage (0-2800mV)
 */
#define AT_READ_ADC "AT+CADC?\r"

/**
 * @brief Single numbering scheme
 * @param mode 0=voice, 2=fax, 4=data
 * @usage AT+CSNS=0 for voice calls
 */
#define AT_SINGLE_NUMBER "AT+CSNS="

/**
 * @brief Reset cell broadcast
 * @usage AT+CDSCB to reset CB settings
 */
#define AT_CB_RESET "AT+CDSCB\r"

/**
 * @brief Configure alternating mode calls
 * @param mode 0=single mode only
 * @usage AT+CMOD=0
 */
#define AT_ALT_MODE "AT+CMOD="

/**
 * @brief Indicate RI when using URC
 * @param status 0=off, 1=on
 * @usage AT+CFGRI=1 to pulse RI on URC
 */
#define AT_RI_INDICATION "AT+CFGRI="

/**
 * @brief Get local timestamp from network
 * @param mode 0=disable, 1=enable
 * @usage AT+CLTS=1 to sync time with network
 */
#define AT_LOCAL_TIMESTAMP "AT+CLTS="

/**
 * @brief External headset jack control
 * @param mode 0=disable URC, 1=enable URC
 * @usage AT+CEXTHS=1 for headset detection
 */
#define AT_HEADSET_JACK "AT+CEXTHS="

/**
 * @brief Headset button status reporting
 * @param mode 0=disable, 1=enable
 * @usage AT+CEXTBUT=1 for button press URC
 */
#define AT_HEADSET_BUTTON "AT+CEXTBUT="

/**
 * @brief SIM inserted status reporting
 * @param n 0=disable, 1=enable
 * @usage AT+CSMINS=1 for SIM detect URC
 */
#define AT_SIM_INSERT "AT+CSMINS="

/**
 * @brief Local DTMF tone generation
 * @param n,DTMF_string Duration and tones
 * @usage AT+CLDTMF=5,"1234" play locally
 */
#define AT_LOCAL_DTMF "AT+CLDTMF="

/**
 * @brief CS voice/data call termination indication
 * @param n 0=disable, 1=enable +CDRIND URC
 * @usage AT+CDRIND=1 for call end notification
 */
#define AT_CALL_END_IND "AT+CDRIND="

/**
 * @brief Get service provider name from SIM
 * @usage AT+CSPN? to read SPN
 */
#define AT_SPN "AT+CSPN?\r"

/**
 * @brief Get/set voice mail number
 * @param vm_number,alpha_string Voice mail info
 * @usage AT+CCVM="+1234567890","Voicemail"
 */
#define AT_VOICEMAIL "AT+CCVM"

/**
 * @brief Get/set mobile operation band
 * @param op_band Band selection
 * @usage AT+CBAND="EGSM_DCS_MODE"
 */
#define AT_BAND "AT+CBAND="

/**
 * @brief Configure hands free operation
 * @param ind,state Enable HF and audio channel
 * @usage AT+CHF=1,2 for hands-free mode
 */
#define AT_HANDSFREE "AT+CHF="

/**
 * @brief Swap audio channels
 * @param n 0=handset, 1=headset, 2=handfree, 3=headset HF
 * @usage AT+CHFA=2 to switch to hands-free
 */
#define AT_AUDIO_CHANNEL "AT+CHFA="

/**
 * @brief Configure slow clock (sleep mode)
 * @param n 0=disable, 1=DTR control, 2=auto
 * @usage AT+CSCLK=2 for automatic sleep
 */
#define AT_SLEEP_MODE "AT+CSCLK="

/**
 * @brief Engineering mode
 * @param mode,Ncell 0=off, 1=on, 2=on with URC, 3=limited
 * @usage AT+CENG=1,1 to show serving and neighbor cells
 */
#define AT_ENGINEERING "AT+CENG="

/**
 * @brief Store class 0 SMS to SIM
 * @param mode 0=disable, 1=enable
 * @usage AT+SCLASS0=1 to store flash SMS
 */
#define AT_CLASS0_SMS "AT+SCLASS0="

/**
 * @brief Show ICCID
 * @usage AT+CCID to read SIM card serial number
 */
#define AT_ICCID "AT+CCID\r"

/**
 * @brief Temperature monitoring
 * @param mode 0=disable, 1=enable
 * @usage AT+CMTE=1 to enable temp monitoring
 */
#define AT_TEMPERATURE "AT+CMTE="

/**
 * @brief Battery temperature query
 * @usage AT+CBTE? to read battery voltage (SIM900D)
 */
#define AT_BATT_TEMP "AT+CBTE?\r"

/**
 * @brief Detect SIM card insertion/removal
 * @param mode 0=disable, 1=enable
 * @usage AT+CSDT=1 for hot-swap detection
 */
#define AT_SIM_DETECT "AT+CSDT="

/**
 * @brief Delete all SMS
 * @param type Delete criteria
 * @usage AT+CMGDA="DEL ALL" to delete all
 */
#define AT_SMS_DELETE_ALL "AT+CMGDA="

/**
 * @brief Play SIM toolkit tone
 * @param mode,tone,duration 0=stop, 1=start
 * @usage AT+STTONE=1,1,500 to play dial tone
 */
#define AT_STK_TONE "AT+STTONE="

/**
 * @brief Generate specific tone
 * @param mode,frequency,periodOn,periodOff,duration
 * @usage AT+SIMTONE=1,2000,500,500,5000
 */
#define AT_GEN_TONE "AT+SIMTONE="

/**
 * @brief Enable/disable alpha string
 * @param mode 0=disable, 1=enable
 * @usage AT+CCPD=1 to show alpha strings
 */
#define AT_ALPHA_STRING "AT+CCPD="

/**
 * @brief Get SIM card group identifier
 * @usage AT+CGID to read GID1 and GID2
 */
#define AT_GROUP_ID "AT+CGID\r"

/**
 * @brief Show mobile originated call state
 * @param mode 0=disable, 1=enable MO RING/CONNECTED
 * @usage AT+MORING=1 for call progress
 */
#define AT_MO_CALL_STATE "AT+MORING="

/**
 * @brief Enable non-ASCII SMS in text mode
 * @param mode 0=normal, 1=enable hex (0x00-0x7F)
 * @usage AT+CMGHEX=1 for special characters
 */
#define AT_SMS_HEX "AT+CMGHEX="

/**
 * @brief Audio channel loopback test
 * @param state,type 0=off, 1=on; 0=normal, 1=aux
 * @usage AT+AUTEST=1,0 for audio test
 */
#define AT_AUDIO_TEST "AT+AUTEST="

/**
 * @brief Configure SMS code mode
 * @param mode 0=Nokia compatible, 1=Siemens compatible
 * @usage AT+CCODE=0
 */
#define AT_SMS_CODE_MODE "AT+CCODE="

/**
 * @brief Enable initial URC presentation
 * @param mode 0=disable, 1=enable "Call Ready" URC
 * @usage AT+CIURC=1 to show startup URCs
 */
#define AT_INITIAL_URC "AT+CIURC="

/**
 * @brief Change PS super password
 * @param oldpwd,newpwd 8-digit passwords
 * @usage AT+CPSPWD="12345678","87654321"
 */
#define AT_SUPER_PASSWORD "AT+CPSPWD="

/**
 * @brief Enable proprietary unsolicited indications
 * @param exunsol,mode "SQ" for signal quality URC
 * @usage AT+EXUNSOL="SQ",1 for RSSI updates
 */
#define AT_PROPRIETARY_URC "AT+EXUNSOL="

/**
 * @brief Change GPRS multislot class
 * @param class 4,8,9,10
 * @usage AT+CGMSCLASS=10 for max throughput
 */
#define AT_MULTISLOT_CLASS "AT+CGMSCLASS="

/**
 * @brief View flash device type
 * @usage AT+CDEVICE? to check flash chip
 */
#define AT_FLASH_DEVICE "AT+CDEVICE?\r"

/**
 * @brief Call ready query
 * @usage AT+CCALR? to check if ready for calls
 */
#define AT_CALL_READY "AT+CCALR?\r"

/**
 * @brief Display product identification (verbose)
 * @usage AT+GSV for full product info
 */
#define AT_PRODUCT_INFO "AT+GSV\r"

/**
 * @brief Control GPIO
 * @param operation,GPIO,function,level
 * @usage AT+SGPIO=0,1,1,1 to set GPIO1 high
 */
#define AT_GPIO_CONTROL "AT+SGPIO="

/**
 * @brief Generate PWM signal
 * @param index,period,level PWM parameters
 * @usage AT+SPWM=1,100,50 for 50% duty cycle
 */
#define AT_PWM_GENERATE "AT+SPWM="

/**
 * @brief Echo cancellation control
 * @param mic,es,ses,mode Echo suppression params
 * @usage AT+ECHO=0,7,5,1 to enable echo cancel
 */
#define AT_ECHO_CANCEL "AT+ECHO="

/**
 * @brief Control auto audio switch
 * @param mode 0=manual, 1=auto with hook, 2=manual with hook
 * @usage AT+CAAS=1 for automatic switching
 */
#define AT_AUTO_AUDIO "AT+CAAS="

/**
 * @brief Configure voice coding type
 * @param voice_rate_coding 0-16, codec priority
 * @usage AT+SVR=16 for full codec support
 */
#define AT_VOICE_CODEC "AT+SVR="

/**
 * @brief Reject incoming call (busy)
 * @param mode 0=accept calls, 1=reject as busy
 * @usage AT+GSMBUSY=1 to auto-reject calls
 */
#define AT_REJECT_CALL "AT+GSMBUSY="

/* ========================================================================== */
/*                        RESPONSE CODES                                      */
/* ========================================================================== */

#define AT_RESPONSE_OK "OK"
#define AT_RESPONSE_ERROR "ERROR"
#define AT_RESPONSE_CONNECT "CONNECT"
#define AT_RESPONSE_RING "RING"
#define AT_RESPONSE_NO_CARRIER "NO CARRIER"
#define AT_RESPONSE_NO_DIALTONE "NO DIALTONE"
#define AT_RESPONSE_BUSY "BUSY"
#define AT_RESPONSE_NO_ANSWER "NO ANSWER"

/* ========================================================================== */
/*                        FUNCTION PROTOTYPES                                 */
/* ========================================================================== */

/**
 * @brief Initialize and configure SIM900A module
 * 
 * @param config Pointer to configuration structure
 * @return esp_err_t ESP_OK on success, error code otherwise
 */
esp_err_t sim900_setup(const sim900_config_t *config);

/**
 * @brief Initialize SIM900A module communication
 * 
 * This function tests communication with the module and waits for it to be ready.
 * It sends AT commands and checks for proper responses.
 * 
 * @return esp_err_t ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t sim900_init(void);

/**
 * @brief Send AT command to SIM900A module
 * 
 * Sends an AT command and waits for response. The response buffer is static
 * and will be overwritten on next call.
 * 
 * @param command AT command string (must include \r terminator)
 * @return char* Pointer to response string, NULL on error
 * 
 * @note The returned pointer is valid until next call to this function
 * @note Caller should not free the returned pointer
 */
char* send_at_command(const char *command);

/**
 * @brief Send AT command with timeout
 * 
 * @param command AT command string
 * @param response Buffer to store response
 * @param response_size Size of response buffer
 * @param timeout_ms Timeout in milliseconds
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sim900_send_command_timeout(const char *command, char *response, 
                                       size_t response_size, uint32_t timeout_ms);

/**
 * @brief Check if module is ready
 * 
 * @return bool true if module responds to AT command
 */
bool sim900_is_ready(void);

/**
 * @brief Deinitialize SIM900A module
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t sim900_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* SIM900_H */