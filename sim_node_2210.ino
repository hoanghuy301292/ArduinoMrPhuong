#define RX_BUFF_SIZE 36
#define MANG_BU "00000000000000000000000000#"

#include "SIM900.h"
#include <SoftwareSerial.h>
#include "sms.h"
#include "call.h"
#include "phonebook_credit.h"
#include "TimerOne.h"

unsigned long previousMillis  = 0;
const long interval = 1000; 
 
SMSGSM sms;
CallGSM call; 

danhBa danhba;
taiKhoan taikhoan;
 
boolean started = false; 
boolean fullBuffer = false;
boolean finishProcess = true; 
byte phongGuiLenh_e, phongGuiLenh_r = 0;
String noiDungTinNhan = "";
char RxSerial3Buffer[RX_BUFF_SIZE]; 
char smstext[160]; 
char number[20];  
char* soDienThoaiCapQuyen[]={"+84398890102", "+84937260622", "+84985008086", "+84773692265", "", "", "", "", "", ""};
char number1[14]="+84985008086";
char number2[14]="+841698890102";
char dktb1[]="dktb123:1n2n3n4n5n";
char dktb2[]="dktb123:1f2f3f4f5f";
byte trangThaiCamBienNhanDuoc[10] = {0};
String lenh1="s";
String lenh2="s";
String lenh3="s";
String lenh4="s";
String lenh5="s";
char lenhSim[9];
void setup(){
  
  Serial3.begin(9600);
  Serial.begin(9600);
  Serial.println("Gui va nhan tin nhan");
  if (gsm.begin(2400))
  {
    Serial.println("\nstatus=READY");
    // sms.SendSMS(soDienThoaiCapQuyen[1],"da khoi dong sim");   
    started=true;  
  }
  else Serial.println("\nstatus=IDLE");

//*** ham nap cac so dt duoc cap quyen vao sim ban dau ****//
  Serial.println("Dang ki so dien thoai duoc cap quyen");
for(byte i = 1;i<11;i++)
{
gsm.WritePhoneNumber(i,soDienThoaiCapQuyen[i-1]);
}
  

 pinMode(13, OUTPUT);
 digitalWrite(13, LOW);
 // for (byte i = 1; i<41; i++)
 //  {
 //    sms.DeleteSMS(byte(i));
 //  }
  Serial.println("Da xoa tin nhan");

 //   if(started){
 //   sms.SendSMS(soDienThoaiCapQuyen[1], "Online");
 // }
  // Timer1.initialize(100000); // ngat 1000000 micros
  // Timer1.attachInterrupt(nhanTinNhan);
  RxSerial3Buffer[RX_BUFF_SIZE - 1] = '\0';
}
 
void loop()
{
   unsigned long currentMillis = millis();

  // Nếu hàm millis() đếm được 1000 milli giây = 1s thì sẽ thay đổi trạng thái LED
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    nhanTinNhan();
  } 
}


// e001xxxxxxxxxxxx

void serialEvent3() {
    // Serial.println(sizeof(RxSerial3Buffer));

  static byte count = 0;
  static boolean writeEnable = false;
  char inChar;

  while (Serial3.available() && fullBuffer == false) 
  {
        // delay(1);
    inChar = Serial3.read();
   
    if((inChar == 'e') | (inChar == 'q') | (inChar == 'c') | (inChar == 'a') | (inChar == 's') | (inChar == 'r'))
    {
      count = 0;
      writeEnable = true;
    }

    if(writeEnable)
    {
      // Serial.println("Bat dau ghi mang");
      if(count <= RX_BUFF_SIZE - 2)
      { 
        RxSerial3Buffer[count] = inChar;
        // Serial.print(count);
        // Serial.print("\t");
        // Serial.println(RxSerial3Buffer[count]);
        count++;
      }
      else
      {
        writeEnable = false;
        // RxSerial3Buffer[RX_BUFF_SIZE - 1] = '\0';
      } 
    }  
    if (inChar == '#')
    {
        fullBuffer = true;
        count = 0;
    }
    RFCommandProcessing();
  }
}


void RFCommandProcessing()
{
  if(fullBuffer)
  {
    Serial.println(RxSerial3Buffer);
    switch (RxSerial3Buffer[0])
    {
      case 'e': 
      // Serial.println("Nhan duoc trang thai cam bien");
                phongGuiLenh_e = 0;
                for (byte i=1; i<=3; i++)
                {
                    phongGuiLenh_e = phongGuiLenh_e * 10 + (RxSerial3Buffer[i]-48);   // -48 la giai ra so acckey
                }
                noiDungTinNhan ="";
                noiDungTinNhan = noiDungTinNhan + "Phong " + phongGuiLenh_e + ": ";

                for (byte i = 0; i <= 9; i++)
                {
                  if(RxSerial3Buffer[i+4] - 48 == 0){
                     noiDungTinNhan = noiDungTinNhan + (i+1) + ", ";
                  }
                  else noiDungTinNhan = noiDungTinNhan + "";
                }
                noiDungTinNhan.toCharArray(smstext, 160);
                Serial.println(smstext);
                sms.SendSMS(soDienThoaiCapQuyen[1],smstext);
                call.Call(soDienThoaiCapQuyen[1]);
                break;
      case 'r':
              // Serial.print("Da dieu khien thiet bi phong");
              phongGuiLenh_r = 0;

              for (byte i=1; i<=3; i++)
              {
                  phongGuiLenh_r = phongGuiLenh_r * 10 + (RxSerial3Buffer[i]-48);   // -48 la giai ra so acckey
              }
              noiDungTinNhan ="";
              noiDungTinNhan = noiDungTinNhan + "Phong " + phongGuiLenh_r + " da dieu khien thiet bi";
              noiDungTinNhan.toCharArray(smstext, 160);
              Serial.println(smstext);
              sms.SendSMS(soDienThoaiCapQuyen[1],smstext);
              break;
      default: break;

    }
    delay(50);
    // memset(RxSerial3Buffer,0, sizeof(RxSerial3Buffer));
    // RxSerial3Buffer[RX_BUFF_SIZE-1]='\0';
    fullBuffer = false;
  }
}

void nhanTinNhan()
{
  if(started){
	// ****************************// nhan ti nhan dieu khien//***************************************//    
    char pos;
    pos = sms.IsSMSPresent(SMS_UNREAD); 
    if((int)pos){
     if(sms.GetAuthorizedSMS(pos, number, smstext, 160, 1, 10) == GETSMS_AUTH_SMS)
     {
        strlwr(smstext);
        Serial.print("So dien thoai: ");          
        Serial.println(number);
        Serial.print("Noi dung tin nhan: ");
        Serial.println(smstext);
        char tinNhanPhanHoi[160];
        
        // PHAN loai tin nhan//
//        //**********************//TIN NHẮN KIỂM TRA TÀI KHOẢN//*********************************//
          if(strstr(smstext,"kiemtrataikhoan") != '\0'){
          taikhoan.kiemTraTaiKhoan(smstext,tinNhanPhanHoi);
          sms.SendSMS(number,tinNhanPhanHoi);
          }       
//       //**********************//TIN NHẮN KIỂM TRA TÀI KHOẢN//*********************************//
//
//        //****************************//TIN NHẮN NẠP TIỀN//*********************************//   

      // cu phap nap tien : naptien:xxxxxxxxxx
      // xxxxxxxx : la 12 con so cua seri tien
         if(strstr(smstext,"naptien") != '\0')
      {    
          taikhoan.napTien(smstext, tinNhanPhanHoi);        
          sms.SendSMS(number, tinNhanPhanHoi);
      }
//       //****************************//TIN NHẮN NẠP TIỀN//*********************************//  

       //**********************//TIN NHẮN KIỂM TRA DANH BẠ//*********************************//
       if(strstr(smstext,"kiemtradanhba") != '\0')
       {
         danhba.kiemTraDanhBa(tinNhanPhanHoi);
         sms.SendSMS(number, tinNhanPhanHoi);
       }      
      //**********************//TIN NHẮN KIỂM TRA DANH BẠ//*********************************//

        //**********************//TIN NHẮN THÊM SỐ VÀO DANH BẠ//******************************//
        // themso:x sdt
        // x = la vi tri nap sdt
        // sdt = +84xxxxxxxxxx
      if(strstr(smstext,"themso") != '\0')
//      && strcmp(number,soDienThoaiCapQuyen[0]) == 0)
      {
          if(danhba.themSoDienThoai(smstext))
          {
            sms.SendSMS(number,"Them so dien thoai thanh cong!");
          }
          else 
          {
            sms.SendSMS(number, "Them so dien thoai khong thanh cong!");
          }
      }
      //**********************//TIN NHẮN THÊM SỐ VÀO DANH BẠ//******************************//

       if(strstr(smstext,"dktb") != '\0')
       {
        /* 
         *  kiem tra cu phap
         */
        int check = 0;
        for(int i = 7;i<18; i++)
        {
          if(smstext[i]==dktb1[i] || smstext[i]==dktb2[i])
          {
           check++;
          }          
        }
        if(check==11){
             Serial.println("DUNG cu phap"); 
          }
          else{
             Serial.println("SAI cu phap"); 
             // smsSend("sai cu phap");
          }
          lenh1="s";
          lenh2="s";
          lenh3="s";
          lenh4="s";
          lenh5="s";
          // "dktb123:1n2n3n4n5n";
         lenh1 = lenh1 + smstext[4] + smstext[5] + smstext[6] + "001" + ((smstext[9]=='n')?"1":"0") + MANG_BU;
         
         lenh2 = lenh2 + smstext[4] + smstext[5] + smstext[6] + "002" + ((smstext[11]=='n')?"1":"0") + MANG_BU;
         
         lenh3 = lenh3 + smstext[4] + smstext[5] + smstext[6] + "003" + ((smstext[13]=='n')?"1":"0") + MANG_BU;
         
         lenh4 = lenh4 + smstext[4] + smstext[5] + smstext[6] + "004" + ((smstext[15]=='n')?"1":"0") + MANG_BU;
         
         lenh5 = lenh5 + smstext[4] + smstext[5] + smstext[6] + "005" + ((smstext[17]=='n')?"1":"0") + MANG_BU;
         
         Serial3.print(lenh1);
         Serial.println(lenh1);
         delay(1500);
         Serial3.print(lenh2);
         Serial.println(lenh2);
         delay(1500);
         Serial3.print(lenh3);
         Serial.println(lenh3);
         delay(1500);
         Serial3.print(lenh4);
         Serial.println(lenh4);
         delay(1500);
         Serial3.print(lenh5);
         Serial.println(lenh5);
         
     
        }
      
    }
  
    

 // ****************************// nhan ti nhan dieu khien//***************************************// 

    sms.DeleteSMS(byte(pos));
  }
}
}
