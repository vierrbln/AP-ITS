s??         f       T   ?   ????                               tsadj       Teststand adjustment panel                    ?  ? ??CAObjHandle  !    UIR settings entries in application.ini file

Mandatory entries:

TitlebarText = Your company name...

ProductName = Product name or project name ...

PanelType = SINGLE 
Valid values: SINGLE

DebugPanelPosition = TOPLEFT
Valid values: TOPLEFT, TOPRIGHT, BOTTOMLEFT, BOTTOMRIGHT

SerialIsMandatory = 1
Valid values: 0 -> Serial number is not mandatory, 1 -> Serial number is mandatory, 2 -> Serial number is mandatory and serial lenght must be verified

SerialLength = 16
If SerialIsMandatory = 2, this tag specify the length of serial number
     w    For use with TestStand: The TestStand sequence context ("ThisContext")

For use in other environments: Must be zero.
     I    Error Flag.
          
Return :
   0 -> No Error
   1 -> Error Occured
     Z    Error code. Negative values indicate an error.

Return:

   0 -> No Error
 < 0 -> Error
     ?    Error message buffer (1024 bytes)

In case of an error, the error message is copied into the error message buffer. The buffer remains unchanged if the function returns successfully.
     p    Pass the pointer to a string containing the logical name or a bench name from the application layer .ini file.     '    Returns the Resource ID of the bench.    ? 3 ?  ?  ?    Sequence Context                  ? ? Q    `    ErrorOccurred                     ? ? ?    `    ErrorCode                         ? ?_    `    ErrorMessage                      ? ? ?    `    Bench Name                        * ?    `    ResourceID                         0    	            	            	                	            w    For use with TestStand: The TestStand sequence context ("ThisContext")

For use in other environments: Must be zero.
     I    Error Flag.
          
Return :
   0 -> No Error
   1 -> Error Occured
     Z    Error code. Negative values indicate an error.

Return:

   0 -> No Error
 < 0 -> Error
     ?    Error message buffer (1024 bytes)

In case of an error, the error message is copied into the error message buffer. The buffer remains unchanged if the function returns successfully.
         Resource Identification

    ? %   ?  ?    Sequence Context                  O 6 
   `    ErrorOccurred                     ? ?    `    ErrorCode                         I    `    ErrorMessage                      ? % ?    `    Resource ID                     ???? ?     ?    LowerLimit                      ???? ? ?    ?    UpperLimit                      ???? %Q    ?    NameOfStep                      ???? ?     ?    Unit                            ???? ? ?    ?    CompType                        ???? ??    ?    Format                          ???? a 0    ?    ButtonText                      ???? ?? 	   ?    AlwaysOnTop                        0    	            	            	                0    0    0    0    0    0    0    0    w    For use with TestStand: The TestStand sequence context ("ThisContext")

For use in other environments: Must be zero.
     I    Error Flag.
          
Return :
   0 -> No Error
   1 -> Error Occured
     Z    Error code. Negative values indicate an error.

Return:

   0 -> No Error
 < 0 -> Error
     ?    Error message buffer (1024 bytes)

In case of an error, the error message is copied into the error message buffer. The buffer remains unchanged if the function returns successfully.
         Resource Identification

    ? %   ?  ?    Sequence Context                  j 6    `    ErrorOccurred                     ? ?    `    ErrorCode                         I    `    ErrorMessage                      ? % ?    `    Resource ID                     ???? ? ?    ?    Value                              0    	            	            	                0    w    For use with TestStand: The TestStand sequence context ("ThisContext")

For use in other environments: Must be zero.
     I    Error Flag.
          
Return :
   0 -> No Error
   1 -> Error Occured
     Z    Error code. Negative values indicate an error.

Return:

   0 -> No Error
 < 0 -> Error
     ?    Error message buffer (1024 bytes)

In case of an error, the error message is copied into the error message buffer. The buffer remains unchanged if the function returns successfully.
     '    Returns the Resource ID of the bench.    p 7 ?  ?  ?    Sequence Context                  ?	 P    `    ErrorOccurred                     @	 ?    `    ErrorCode                         ?	^    `    ErrorMessage                      b ?    `    ResourceID                         0    	            	            	                w    For use with TestStand: The TestStand sequence context ("ThisContext")

For use in other environments: Must be zero.
     I    Error Flag.
          
Return :
   0 -> No Error
   1 -> Error Occured
     Z    Error code. Negative values indicate an error.

Return:

   0 -> No Error
 < 0 -> Error
     ?    Error message buffer (1024 bytes)

In case of an error, the error message is copied into the error message buffer. The buffer remains unchanged if the function returns successfully.
         Resource Identification

    ? 7 ?  ?  ?    Sequence Context                  F	 G    `    ErrorOccurred                     ?	 ?    `    ErrorCode                         ?	U    `    ErrorMessage                      ? ? ?    `    Resource ID                        0    	            	            	             ????          ?  Y     K.    Setup                           ????       ????  ?     K.    DisplayAdjustmentPanel          ????       ????        K.    SetValueAdjustmentPanel         ????       ????  ?     K.    HideAdjustmentPanel             ????       ????  ?     K.    Cleanup                             ????                                     DSetup                                DDisplay Adjustment Panel             DSet Value Adjustment Panel           DHide Adjustment panel                DCleanup                         