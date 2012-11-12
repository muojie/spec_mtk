void OTM8009A_HSD043_cpu16bit_Initial(void)
{
WriteCtrl(0xff00); WriteData(0x80);
WriteCtrl(0xff01); WriteData(0x09); //enable EXTC
WriteCtrl(0xff02); WriteData(0x01);
WriteCtrl(0xff80); WriteData(0x80); //enable Orise mode
WriteCtrl(0xff81); WriteData(0x09);
WriteCtrl(0xff03); WriteData(0x01); //enable SPI+I2C cmd2 read

//gamma DC
WriteCtrl(0xc0b4); WriteData(0x50); //column inversion 	
WriteCtrl(0xC489); WriteData(0x08); //reg off	
WriteCtrl(0xC0a3); WriteData(0x00); //pre-charge //V02             
WriteCtrl(0xC582); WriteData(0xA3); //REG-pump23
WriteCtrl(0xC590); WriteData(0x96); //Pump setting (3x=D6)-->(2x=96)//v02 01/11
WriteCtrl(0xC591); WriteData(0x87); //Pump setting(VGH/VGL)   
WriteCtrl(0xD800); WriteData(0x73); //GVDD=4.5V  73   
WriteCtrl(0xD801); WriteData(0x71); //NGVDD=4.5V 71  

//VCOMDC                                                                 
WriteCtrl(0xD900); WriteData(0x5C); // VCOMDC=   
Delayms(20);
  
// Positive
WriteCtrl(0xE100); WriteData(0x09);
WriteCtrl(0xE101); WriteData(0x0a);
WriteCtrl(0xE102); WriteData(0x0e);
WriteCtrl(0xE103); WriteData(0x0d);
WriteCtrl(0xE104); WriteData(0x07);
WriteCtrl(0xE105); WriteData(0x18);
WriteCtrl(0xE106); WriteData(0x0d);
WriteCtrl(0xE107); WriteData(0x0d);
WriteCtrl(0xE108); WriteData(0x01);
WriteCtrl(0xE109); WriteData(0x04);
WriteCtrl(0xE10A); WriteData(0x05);
WriteCtrl(0xE10B); WriteData(0x06);
WriteCtrl(0xE10C); WriteData(0x0e);
WriteCtrl(0xE10D); WriteData(0x25);
WriteCtrl(0xE10E); WriteData(0x22);
WriteCtrl(0xE10F); WriteData(0x05);

// Negative
WriteCtrl(0xE200); WriteData(0x09);
WriteCtrl(0xE201); WriteData(0x0a);
WriteCtrl(0xE202); WriteData(0x0e);
WriteCtrl(0xE203); WriteData(0x0d);
WriteCtrl(0xE204); WriteData(0x07);
WriteCtrl(0xE205); WriteData(0x18);
WriteCtrl(0xE206); WriteData(0x0d);
WriteCtrl(0xE207); WriteData(0x0d);
WriteCtrl(0xE208); WriteData(0x01);
WriteCtrl(0xE209); WriteData(0x04);
WriteCtrl(0xE20A); WriteData(0x05);
WriteCtrl(0xE20B); WriteData(0x06);
WriteCtrl(0xE20C); WriteData(0x0e);
WriteCtrl(0xE20D); WriteData(0x25);
WriteCtrl(0xE20E); WriteData(0x22);
WriteCtrl(0xE20F); WriteData(0x05);

WriteCtrl(0xC181); WriteData(0x66);     //Frame rate 65Hz//V02   

// RGB I/F setting VSYNC for OTM8018 0x0e
WriteCtrl(0xC1a1); WriteData(0x08);     //external Vsync,Hsync,DE
WriteCtrl(0xC0a3); WriteData(0x1b);     //pre-charge //V02
WriteCtrl(0xC481); WriteData(0x83);     //source bias //V02
WriteCtrl(0xC592); WriteData(0x01);     //Pump45
WriteCtrl(0xC5B1); WriteData(0xA9);     //DC voltage setting ;[0]GVDD output, default: 0xa8

// CE8x : vst1, vst2, vst3, vst4
WriteCtrl(0xCE80); WriteData(0x85);	// ce81[7:0] : vst1_shift[7:0]
WriteCtrl(0xCE81); WriteData(0x03);	// ce82[7:0] : 0000,	vst1_width[3:0]
WriteCtrl(0xCE82); WriteData(0x00);	// ce83[7:0] : vst1_tchop[7:0]
WriteCtrl(0xCE83); WriteData(0x84);	// ce84[7:0] : vst2_shift[7:0]
WriteCtrl(0xCE84); WriteData(0x03);	// ce85[7:0] : 0000,	vst2_width[3:0]
WriteCtrl(0xCE85); WriteData(0x00);	// ce86[7:0] : vst2_tchop[7:0]	                                                                                                                      
WriteCtrl(0xCE86); WriteData(0x83);	// ce87[7:0] : vst3_shift[7:0]	                                                                                                                      
WriteCtrl(0xCE87); WriteData(0x03);	// ce88[7:0] : 0000,	vst3_width[3:0]
WriteCtrl(0xCE88); WriteData(0x00);	// ce89[7:0] : vst3_tchop[7:0]                                                     
WriteCtrl(0xCE89); WriteData(0x82);	// ce8a[7:0] : vst4_shift[7:0]
WriteCtrl(0xCE8a); WriteData(0x03);	// ce8b[7:0] : 0000,	vst4_width[3:0]
WriteCtrl(0xCE8b); WriteData(0x00);	// ce8c[7:0] : vst4_tchop[7:0]

//CEAx : clka1, clka2
WriteCtrl(0xCEa0); WriteData(0x38);	// cea1[7:0] : clka1_width[3:0], clka1_shift[11:8]
WriteCtrl(0xCEa1); WriteData(0x02);	// cea2[7:0] : clka1_shift[7:0]
WriteCtrl(0xCEa2); WriteData(0x03);	// cea3[7:0] : clka1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]                                                
WriteCtrl(0xCEa3); WriteData(0x21);	// cea4[7:0] : clka1_switch[7:0]                                                                                        
WriteCtrl(0xCEa4); WriteData(0x00);	// cea5[7:0] : clka1_extend[7:0]                                                                                        
WriteCtrl(0xCEa5); WriteData(0x00);	// cea6[7:0] : clka1_tchop[7:0]                                                                                         
WriteCtrl(0xCEa6); WriteData(0x00);	// cea7[7:0] : clka1_tglue[7:0]                                                                                         
WriteCtrl(0xCEa7); WriteData(0x38);	// cea8[7:0] : clka2_width[3:0], clka2_shift[11:8]                                                                      
WriteCtrl(0xCEa8); WriteData(0x01);	// cea9[7:0] : clka2_shift[7:0]                                                                                         
WriteCtrl(0xCEa9); WriteData(0x03);	// ceaa[7:0] : clka2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]                                                
WriteCtrl(0xCEaa); WriteData(0x22);	// ceab[7:0] : clka2_switch[7:0]                                                                                        
WriteCtrl(0xCEab); WriteData(0x00);	// ceac[7:0] : clka2_extend                                                                                             
WriteCtrl(0xCEac); WriteData(0x00);	// cead[7:0] : clka2_tchop                                                                                              
WriteCtrl(0xCEad); WriteData(0x00);	// ceae[7:0] : clka2_tglue                                                                                              
                                                                                                                                                            
//CEBx : clka3, clka4                                                                                                                                       
WriteCtrl(0xCEb0); WriteData(0x38);	// ceb1[7:0] : clka3_width[3:0], clka3_shift[11:8]                                                                      
WriteCtrl(0xCEb1); WriteData(0x00);	// ceb2[7:0] : clka3_shift[7:0]                                                                                         
WriteCtrl(0xCEb2); WriteData(0x03);	// ceb3[7:0] : clka3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]                                                
WriteCtrl(0xCEb3); WriteData(0x23);	// ceb4[7:0] : clka3_switch[7:0]                                                                                        
WriteCtrl(0xCEb4); WriteData(0x00);	// ceb5[7:0] : clka3_extend[7:0]                                                                                        
WriteCtrl(0xCEb5); WriteData(0x00);	// ceb6[7:0] : clka3_tchop[7:0]                                                                                         
WriteCtrl(0xCEb6); WriteData(0x00);	// ceb7[7:0] : clka3_tglue[7:0]                                                                                         
WriteCtrl(0xCEb7); WriteData(0x30);	// ceb8[7:0] : clka4_width[3:0], clka2_shift[11:8]                                                                      
WriteCtrl(0xCEb8); WriteData(0x00);	// ceb9[7:0] : clka4_shift[7:0]                                                                                         
WriteCtrl(0xCEb9); WriteData(0x03);	// ceba[7:0] : clka4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]                                                
WriteCtrl(0xCEba); WriteData(0x24);	// cebb[7:0] : clka4_switch[7:0]                                                                                        
WriteCtrl(0xCEbb); WriteData(0x00);	// cebc[7:0] : clka4_extend                                                                                             
WriteCtrl(0xCEbc); WriteData(0x00);	// cebd[7:0] : clka4_tchop                                                                                              
WriteCtrl(0xCEbd); WriteData(0x00);	// cebe[7:0] : clka4_tglue                                                                                              
                                                                                                                                                            
//CECx : clkb1, clkb2                                                                                                                                       
WriteCtrl(0xCEc0); WriteData(0x30);	// cec1[7:0] : clkb1_width[3:0], clkb1_shift[11:8]                                                                      
WriteCtrl(0xCEc1); WriteData(0x01);	// cec2[7:0] : clkb1_shift[7:0]                                                                                         
WriteCtrl(0xCEc2); WriteData(0x03);	// cec3[7:0] : clkb1_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]                                                
WriteCtrl(0xCEc3); WriteData(0x25);	// cec4[7:0] : clkb1_switch[7:0]                                                                                        
WriteCtrl(0xCEc4); WriteData(0x00);	// cec5[7:0] : clkb1_extend[7:0]                                                                                        
WriteCtrl(0xCEc5); WriteData(0x00);	// cec6[7:0] : clkb1_tchop[7:0]                                                                                         
WriteCtrl(0xCEc6); WriteData(0x00);	// cec7[7:0] : clkb1_tglue[7:0]                                                                                         
WriteCtrl(0xCEc7); WriteData(0x30);	// cec8[7:0] : clkb2_width[3:0], clkb2_shift[11:8]                                                                      
WriteCtrl(0xCEc8); WriteData(0x02);	// cec9[7:0] : clkb2_shift[7:0]                                                                                         
WriteCtrl(0xCEc9); WriteData(0x03);	// ceca[7:0] : clkb2_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]                                                
WriteCtrl(0xCEca); WriteData(0x26);	// cecb[7:0] : clkb2_switch[7:0]                                                                                        
WriteCtrl(0xCEcb); WriteData(0x00);	// cecc[7:0] : clkb2_extend                                                                                             
WriteCtrl(0xCEcc); WriteData(0x00);	// cecd[7:0] : clkb2_tchop                                                                                              
WriteCtrl(0xCEcd); WriteData(0x00);	// cece[7:0] : clkb2_tglue                                                                                              
                                   	                                                                                                                        
//CEDx : clkb3, clkb4                                                                                                                                       
WriteCtrl(0xCEd0); WriteData(0x30);	// ced1[7:0] : clkb3_width[3:0], clkb3_shift[11:8]                                                                      
WriteCtrl(0xCEd1); WriteData(0x03);	// ced2[7:0] : clkb3_shift[7:0]                                                                                         
WriteCtrl(0xCEd2); WriteData(0x03);	// ced3[7:0] : clkb3_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]                                                
WriteCtrl(0xCEd3); WriteData(0x27);	// ced4[7:0] : clkb3_switch[7:0]                                                                                        
WriteCtrl(0xCEd4); WriteData(0x00);	// ced5[7:0] : clkb3_extend[7:0]                                                                                        
WriteCtrl(0xCEd5); WriteData(0x00);	// ced6[7:0] : clkb3_tchop[7:0]                                                                                         
WriteCtrl(0xCEd6); WriteData(0x00);	// ced7[7:0] : clkb3_tglue[7:0]                                                                                         
WriteCtrl(0xCEd7); WriteData(0x30);	// ced8[7:0] : clkb4_width[3:0], clkb4_shift[11:8]                                                                      
WriteCtrl(0xCEd8); WriteData(0x04);	// ced9[7:0] : clkb4_shift[7:0]                                                                                         
WriteCtrl(0xCEd9); WriteData(0x03);	// ceda[7:0] : clkb4_sw_tg, odd_high, flat_head, flat_tail, switch[11:8]                                                
WriteCtrl(0xCEda); WriteData(0x28);	// cedb[7:0] : clkb4_switch[7:0]                                                                                        
WriteCtrl(0xCEdb); WriteData(0x00);	// cedc[7:0] : clkb4_extend                                                                                             
WriteCtrl(0xCEdc); WriteData(0x00);	// cedd[7:0] : clkb4_tchop                                                                                              
WriteCtrl(0xCEdd); WriteData(0x00);	// cede[7:0] : clkb4_tglue                                                                                              
                                                                                                                                                            
//CFCx :                                                                                                                                                    
WriteCtrl(0xCFc0); WriteData(0x00);	// cfc1[7:0] : eclk_normal_width[7:0]                                                                                   
WriteCtrl(0xCFc1); WriteData(0x00);	// cfc2[7:0] : eclk_partial_width[7:0]                                                                                  
WriteCtrl(0xCFc2); WriteData(0x00);	// cfc3[7:0] : all_normal_tchop[7:0]                                                                                    
WriteCtrl(0xCFc3); WriteData(0x00);	// cfc4[7:0] : all_partial_tchop[7:0]                                                                                   
WriteCtrl(0xCFc4); WriteData(0x00);	// cfc5[7:0] : eclk1_follow[3:0], eclk2_follow[3:0]                                                                     
WriteCtrl(0xCFc5); WriteData(0x00);	// cfc6[7:0] : eclk3_follow[3:0], eclk4_follow[3:0]                                                                     
WriteCtrl(0xCFc6); WriteData(0x00);	// cfc7[7:0] : 00, vstmask, vendmask, 00, dir1, dir2 (0=VGL, 1=VGH)                                                     
WriteCtrl(0xCFc7); WriteData(0x00);	// cfc8[7:0] : reg_goa_gnd_opt, reg_goa_dpgm_tail_set, reg_goa_f_gating_en, reg_goa_f_odd_gating, toggle_mod1, 2, 3, 4  
WriteCtrl(0xCFc8); WriteData(0x00);	// cfc9[7:0] : duty_block[3:0], DGPM[3:0]                                                                               
WriteCtrl(0xCFc9); WriteData(0x00);	// cfca[7:0] : reg_goa_gnd_period[7:0]                                                                                  

//CFDx :
WriteCtrl(0xCFd0);	WriteData(0x00);// cfd1[7:0] : 0000000, reg_goa_frame_odd_high
// Parameter 1

//--------------------------------------------------------------------------------
//		initial setting 3 < Panel setting >
//--------------------------------------------------------------------------------
// cbcx
WriteCtrl(0xCBc0); WriteData(0x00);	//cbc1[7:0] : enmode H-byte of sig1  (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBc1); WriteData(0x00);	//cbc2[7:0] : enmode H-byte of sig2  (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBc2); WriteData(0x00);	//cbc3[7:0] : enmode H-byte of sig3  (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBc3); WriteData(0x00);	//cbc4[7:0] : enmode H-byte of sig4  (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBc4); WriteData(0x04);	//cbc5[7:0] : enmode H-byte of sig5  (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBc5); WriteData(0x04);	//cbc6[7:0] : enmode H-byte of sig6  (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBc6); WriteData(0x04);	//cbc7[7:0] : enmode H-byte of sig7  (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBc7); WriteData(0x04);	//cbc8[7:0] : enmode H-byte of sig8  (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBc8); WriteData(0x04);	//cbc9[7:0] : enmode H-byte of sig9  (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBc9); WriteData(0x04);	//cbca[7:0] : enmode H-byte of sig10 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBca); WriteData(0x00);	//cbcb[7:0] : enmode H-byte of sig11 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBcb); WriteData(0x00);	//cbcc[7:0] : enmode H-byte of sig12 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBcc); WriteData(0x00);	//cbcd[7:0] : enmode H-byte of sig13 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBcd); WriteData(0x00);	//cbce[7:0] : enmode H-byte of sig14 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBce); WriteData(0x00);	//cbcf[7:0] : enmode H-byte of sig15 (pwrof_0, pwrof_1, norm, pwron_4 )                          
                                                                                                                                     
// cbdx                                                                                                                              
WriteCtrl(0xCBd0); WriteData(0x00);	//cbd1[7:0] : enmode H-byte of sig16 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBd1); WriteData(0x00);	//cbd2[7:0] : enmode H-byte of sig17 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBd2); WriteData(0x00);	//cbd3[7:0] : enmode H-byte of sig18 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBd3); WriteData(0x00);	//cbd4[7:0] : enmode H-byte of sig19 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBd4); WriteData(0x00);	//cbd5[7:0] : enmode H-byte of sig20 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBd5); WriteData(0x00);	//cbd6[7:0] : enmode H-byte of sig21 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBd6); WriteData(0x00);	//cbd7[7:0] : enmode H-byte of sig22 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBd7); WriteData(0x00);	//cbd8[7:0] : enmode H-byte of sig23 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBd8); WriteData(0x00);	//cbd9[7:0] : enmode H-byte of sig24 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBd9); WriteData(0x04);	//cbda[7:0] : enmode H-byte of sig25 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBda); WriteData(0x04);	//cbdb[7:0] : enmode H-byte of sig26 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBdb); WriteData(0x04);	//cbdc[7:0] : enmode H-byte of sig27 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBdc); WriteData(0x04);	//cbdd[7:0] : enmode H-byte of sig28 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBdd); WriteData(0x04);	//cbde[7:0] : enmode H-byte of sig29 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBde); WriteData(0x04);	//cbdf[7:0] : enmode H-byte of sig30 (pwrof_0, pwrof_1, norm, pwron_4 )                          
                                                             
// cbex                                                      
WriteCtrl(0xCBe0); WriteData(0x00);	//cbe1[7:0] : enmode H-byte of sig31 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBe1); WriteData(0x00);	//cbe2[7:0] : enmode H-byte of sig32 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBe2); WriteData(0x00);	//cbe3[7:0] : enmode H-byte of sig33 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBe3); WriteData(0x00);	//cbe4[7:0] : enmode H-byte of sig34 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBe4); WriteData(0x00);	//cbe5[7:0] : enmode H-byte of sig35 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBe5); WriteData(0x00);	//cbe6[7:0] : enmode H-byte of sig36 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBe6); WriteData(0x00);	//cbe7[7:0] : enmode H-byte of sig37 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBe7); WriteData(0x00);	//cbe8[7:0] : enmode H-byte of sig38 (pwrof_0, pwrof_1, norm, pwron_4 )                          
WriteCtrl(0xCBe8); WriteData(0x00);	//cbe9[7:0] : enmode H-byte of sig39 (pwrof_0, pwrof_1, norm, pwron_4 )                                                                                                  
WriteCtrl(0xCBe9); WriteData(0x00);	//cbea[7:0] : enmode H-byte of sig40 (pwrof_0, pwrof_1, norm, pwron_4 )                                                                                                  
                                                  
// cc8x                                           
WriteCtrl(0xCC80); WriteData(0x00);	//cc81[7:0] : reg setting for signal01 selection with u2d mode                                   
WriteCtrl(0xCC81); WriteData(0x00);	//cc82[7:0] : reg setting for signal02 selection with u2d mode                                   
WriteCtrl(0xCC82); WriteData(0x00);	//cc83[7:0] : reg setting for signal03 selection with u2d mode                                   
WriteCtrl(0xCC83); WriteData(0x00);	//cc84[7:0] : reg setting for signal04 selection with u2d mode                                   
WriteCtrl(0xCC84); WriteData(0x0C);	//cc85[7:0] : reg setting for signal05 selection with u2d mode                                   
WriteCtrl(0xCC85); WriteData(0x0A);	//cc86[7:0] : reg setting for signal06 selection with u2d mode                                   
WriteCtrl(0xCC86); WriteData(0x10);	//cc87[7:0] : reg setting for signal07 selection with u2d mode                                   
WriteCtrl(0xCC87); WriteData(0x0E);	//cc88[7:0] : reg setting for signal08 selection with u2d mode                                   
WriteCtrl(0xCC88); WriteData(0x03);	//cc89[7:0] : reg setting for signal09 selection with u2d mode                                   
WriteCtrl(0xCC89); WriteData(0x04);	//cc8a[7:0] : reg setting for signal10 selection with u2d mode                                   
                                                                                                                                     
// cc9x                                                                                                                              
WriteCtrl(0xCC90); WriteData(0x00);	//cc91[7:0] : reg setting for signal11 selection with u2d mode                                   
WriteCtrl(0xCC91); WriteData(0x00);	//cc92[7:0] : reg setting for signal12 selection with u2d mode                                   
WriteCtrl(0xCC92); WriteData(0x00);	//cc93[7:0] : reg setting for signal13 selection with u2d mode                                   
WriteCtrl(0xCC93); WriteData(0x00);	//cc94[7:0] : reg setting for signal14 selection with u2d mode                                   
WriteCtrl(0xCC94); WriteData(0x00);	//cc95[7:0] : reg setting for signal15 selection with u2d mode                                   
WriteCtrl(0xCC95); WriteData(0x00);	//cc96[7:0] : reg setting for signal16 selection with u2d mode                                   
WriteCtrl(0xCC96); WriteData(0x00);	//cc97[7:0] : reg setting for signal17 selection with u2d mode                                   
WriteCtrl(0xCC97); WriteData(0x00);	//cc98[7:0] : reg setting for signal18 selection with u2d mode                                   
WriteCtrl(0xCC98); WriteData(0x00);	//cc99[7:0] : reg setting for signal19 selection with u2d mode                                   
WriteCtrl(0xCC99); WriteData(0x00);	//cc9a[7:0] : reg setting for signal20 selection with u2d mode                                   
WriteCtrl(0xCC9a); WriteData(0x00);	//cc9b[7:0] : reg setting for signal21 selection with u2d mode                                   
WriteCtrl(0xCC9b); WriteData(0x00);	//cc9c[7:0] : reg setting for signal22 selection with u2d mode                                   
WriteCtrl(0xCC9c); WriteData(0x00);	//cc9d[7:0] : reg setting for signal23 selection with u2d mode                                   
WriteCtrl(0xCC9d); WriteData(0x00);	//cc9e[7:0] : reg setting for signal24 selection with u2d mode                                   
WriteCtrl(0xCC9e); WriteData(0x0B);	//cc9f[7:0] : reg setting for signal25 selection with u2d mode                                   
                                                                                                                                     
// ccax                                                                                                                              
WriteCtrl(0xCCa0); WriteData(0x09);	//cca1[7:0] : reg setting for signal26 selection with u2d mode                                   
WriteCtrl(0xCCa1); WriteData(0x0F);	//cca2[7:0] : reg setting for signal27 selection with u2d mode                                   
WriteCtrl(0xCCa2); WriteData(0x0D);	//cca3[7:0] : reg setting for signal28 selection with u2d mode                                   
WriteCtrl(0xCCa3); WriteData(0x01);	//cca4[7:0] : reg setting for signal29 selection with u2d mode                                   
WriteCtrl(0xCCa4); WriteData(0x02);	//cca5[7:0] : reg setting for signal20 selection with u2d mode                                   
WriteCtrl(0xCCa5); WriteData(0x00);	//cca6[7:0] : reg setting for signal31 selection with u2d mode                                   
WriteCtrl(0xCCa6); WriteData(0x00);	//cca7[7:0] : reg setting for signal32 selection with u2d mode                                   
WriteCtrl(0xCCa7); WriteData(0x00);	//cca8[7:0] : reg setting for signal33 selection with u2d mode                                   
WriteCtrl(0xCCa8); WriteData(0x00);	//cca9[7:0] : reg setting for signal34 selection with u2d mode                                   
WriteCtrl(0xCCa9); WriteData(0x00);	//ccaa[7:0] : reg setting for signal35 selection with u2d mode                                   
WriteCtrl(0xCCaa); WriteData(0x00);	//ccab[7:0] : reg setting for signal36 selection with u2d mode                                   
WriteCtrl(0xCCab); WriteData(0x00);	//ccac[7:0] : reg setting for signal37 selection with u2d mode                                   
WriteCtrl(0xCCac); WriteData(0x00);	//ccad[7:0] : reg setting for signal38 selection with u2d mode                                   
WriteCtrl(0xCCad); WriteData(0x00);	//ccae[7:0] : reg setting for signal39 selection with u2d mode                                   
WriteCtrl(0xCCae); WriteData(0x00);	//ccaf[7:0] : reg setting for signal40 selection with u2d mode                                   
                                                                                                                                     
// ccbx                                                                                                                              
WriteCtrl(0xCCb0); WriteData(0x00);	//ccb1[7:0] : reg setting for signal01 selection with d2u mode                                   
WriteCtrl(0xCCb1); WriteData(0x00);	//ccb2[7:0] : reg setting for signal02 selection with d2u mode                                   
WriteCtrl(0xCCb2); WriteData(0x00);	//ccb3[7:0] : reg setting for signal03 selection with d2u mode                                   
WriteCtrl(0xCCb3); WriteData(0x00);	//ccb4[7:0] : reg setting for signal04 selection with d2u mode                                   
WriteCtrl(0xCCb4); WriteData(0x0D);	//ccb5[7:0] : reg setting for signal05 selection with d2u mode                                   
WriteCtrl(0xCCb5); WriteData(0x0F);	//ccb6[7:0] : reg setting for signal06 selection with d2u mode                                   
WriteCtrl(0xCCb6); WriteData(0x09);	//ccb7[7:0] : reg setting for signal07 selection with d2u mode                                   
WriteCtrl(0xCCb7); WriteData(0x0B);	//ccb8[7:0] : reg setting for signal08 selection with d2u mode                                   
WriteCtrl(0xCCb8); WriteData(0x02);	//ccb9[7:0] : reg setting for signal09 selection with d2u mode                                   
WriteCtrl(0xCCb9); WriteData(0x01);	//ccba[7:0] : reg setting for signal10 selection with d2u mode                                   
                                                                                                                                     
// cccx                                                                                                                              
WriteCtrl(0xCCc0); WriteData(0x00);	//ccc1[7:0] : reg setting for signal11 selection with d2u mode                                   
WriteCtrl(0xCCc1); WriteData(0x00);	//ccc2[7:0] : reg setting for signal12 selection with d2u mode                                   
WriteCtrl(0xCCc2); WriteData(0x00);	//ccc3[7:0] : reg setting for signal13 selection with d2u mode                                   
WriteCtrl(0xCCc3); WriteData(0x00);	//ccc4[7:0] : reg setting for signal14 selection with d2u mode                                   
WriteCtrl(0xCCc4); WriteData(0x00);	//ccc5[7:0] : reg setting for signal15 selection with d2u mode                                   
WriteCtrl(0xCCc5); WriteData(0x00);	//ccc6[7:0] : reg setting for signal16 selection with d2u mode                                   
WriteCtrl(0xCCc6); WriteData(0x00);	//ccc7[7:0] : reg setting for signal17 selection with d2u mode                                   
WriteCtrl(0xCCc7); WriteData(0x00);	//ccc8[7:0] : reg setting for signal18 selection with d2u mode                                   
WriteCtrl(0xCCc8); WriteData(0x00);	//ccc9[7:0] : reg setting for signal19 selection with d2u mode                                   
WriteCtrl(0xCCc9); WriteData(0x00);	//ccca[7:0] : reg setting for signal20 selection with d2u mode                                   
WriteCtrl(0xCCca); WriteData(0x00);	//cccb[7:0] : reg setting for signal21 selection with d2u mode                                   
WriteCtrl(0xCCcb); WriteData(0x00);	//cccc[7:0] : reg setting for signal22 selection with d2u mode                                   
WriteCtrl(0xCCcc); WriteData(0x00);	//cccd[7:0] : reg setting for signal23 selection with d2u mode                                   
WriteCtrl(0xCCcd); WriteData(0x00);	//ccce[7:0] : reg setting for signal24 selection with d2u mode                                   
WriteCtrl(0xCCce); WriteData(0x0E);	//cccf[7:0] : reg setting for signal25 selection with d2u mode                                   
                                                                                                                                     
// ccdx                                                                                                                              
WriteCtrl(0xCCd0); WriteData(0x10);	//ccd1[7:0] : reg setting for signal26 selection with d2u mode                                   
WriteCtrl(0xCCd1); WriteData(0x0A);	//ccd2[7:0] : reg setting for signal27 selection with d2u mode                                   
WriteCtrl(0xCCd2); WriteData(0x0C);	//ccd3[7:0] : reg setting for signal28 selection with d2u mode                                   
WriteCtrl(0xCCd3); WriteData(0x04);	//ccd4[7:0] : reg setting for signal29 selection with d2u mode                                   
WriteCtrl(0xCCd4); WriteData(0x03);	//ccd5[7:0] : reg setting for signal30 selection with d2u mode                                   
WriteCtrl(0xCCd5); WriteData(0x00);	//ccd6[7:0] : reg setting for signal31 selection with d2u mode                                   
WriteCtrl(0xCCd6); WriteData(0x00);	//ccd7[7:0] : reg setting for signal32 selection with d2u mode                                   
WriteCtrl(0xCCd7); WriteData(0x00);	//ccd8[7:0] : reg setting for signal33 selection with d2u mode                                   
WriteCtrl(0xCCd8); WriteData(0x00);	//ccd9[7:0] : reg setting for signal34 selection with d2u mode                                   
WriteCtrl(0xCCd9); WriteData(0x00);	//ccda[7:0] : reg setting for signal35 selection with d2u mode                                   
WriteCtrl(0xCCda); WriteData(0x00);	//ccdb[7:0] : reg setting for signal36 selection with d2u mode                                   
WriteCtrl(0xCCdb); WriteData(0x00);	//ccdc[7:0] : reg setting for signal37 selection with d2u mode                                   
WriteCtrl(0xCCdc); WriteData(0x00);	//ccdd[7:0] : reg setting for signal38 selection with d2u mode                                   
WriteCtrl(0xCCdd); WriteData(0x00);	//ccde[7:0] : reg setting for signal39 selection with d2u mode                    
WriteCtrl(0xCCde); WriteData(0x00);	//ccdf[7:0] : reg setting for signal40 selection with d2u mode                    

///=============================
WriteCtrl(0x3A00); WriteData(0x77);     //MCU 16bits D[17:0]
WriteCtrl(0x1100);
delayms(150);	
WriteCtrl(0x2900);
delayms(200);
WriteCtrl(0x2c00);

}