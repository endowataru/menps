
SECTIONS
{
    .dsm_data : ALIGN(0x1000)
    {
        _dsm_data_begin = .;
        
        *(.dsm_data)
        
        . = ALIGN(0x1000);
        
        _dsm_data_end = .;
    }
    
}

/* Insert .dsm_data after .ldata
   (the last section before any debugging sections). */
INSERT BEFORE .stab;

