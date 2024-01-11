<table style="border-collapse: collapse; border: none;">
  <tr style="border-collapse: collapse; border: none;">
    <td style="border-collapse: collapse; border: none;">
      <a href="http://www.openairinterface.org/">
         <img src="./images/oai_final_logo.png" alt="" border=3 height=50 width=150>
         </img>
      </a>
    </td>
    <td style="border-collapse: collapse; border: none; vertical-align: center;">
      <b><font size = "5">List of COTS UEs Tested with OAI</font></b>
    </td>
  </tr>
</table>

|Phone                 |Android Version                                           |PLMN |Remark                                                                                  |
|----------------------|----------------------------------------------------------|-----|----------------------------------------------------------------------------------------|
|Pixel 5               |11, RQ1C.210205.006, Feb 2021, Google Fi, T-Mobile, Sprint|00101|NA                                                                                      |
|Pixel 6               |12                                                        |00101|NA                                                                                      |
|Pixel 6               |13                                                        |00101|issues with continuous connectivity, only connected for ~4 mins. Investigating the issue|
|Pixel 7               |13                                                        |00101|issues with continuous connectivity, only connected for ~4 mins. Investigating the issue|
|Huawei P40 and P40 Pro|                                                          |50501|Did not manage to connect with 00101 PLMN                                               |
|OnePlus 8             |11 Oxygen OS 11.0.2.2.IN21AA                              |00101|NA                                                                                      |
|OnePlus 9             |                                                          |00101|NA                                                                                      |
|OnePlus Nord (ac2003) |12                                                        |00101|NA                                                                                      |
|Oppo Reno7 Pro        |11 and 12                                                 |00101|NA                                                                                      |
|iPhone 14 and 14 Pro  |iOS 16.4.1                                                |00101|NA                                                                                      |
|Quectel               |RM500Q-GL, RM520-GL, RM502Q-AE, RM500U-CN                 |     |NA                                                                                      |
|Simcom                |SIMCOM_SIM8200EA-M2                                       |     |NA                                                                                      |
|Fritzbox 6850         |                                                          |     |NA                                                                                      |


**Sim Cards**

- Opencells
- Symocom SJS1 and SJA2

**Note**:

1. We are having issues with Android 13 we are investigating and trying to resolve them.
2. We have issues with Anritsu Test UICC TM P0551A sim cards because they sent concealed SUCI which is not yet implemented.
