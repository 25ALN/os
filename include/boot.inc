    ;-------------	 loader和kernel   ----------
LOADER_BASE_ADDR equ 0x900  ;表示要载入的二级引导/内核在硬盘上的起始 LBA 扇区号
LOADER_START_SECTOR equ 0x2 ;表示把被读取的扇区写入内存的目标地址偏移/缓冲地址
PAGE_DIR_TABLE_POS equ 0x100000          ;页目录表在内存中的起始位置——从1M开始的位置
;接下来将gdt表中的描述符用宏进行定

DESC_G_4K   equ 1_00000000000000000000000b        ;设置段界限的单位为4KB 
DESC_D_32   equ  1_0000000000000000000000b        ;设置代码段/数据段的有效地址（段内偏移地址）及操作数大小为32位，而非16位 
DESC_L      equ   0_000000000000000000000b
DESC_AVL    equ        0_00000000000000000000b

DESC_LIMIT_CODE2    equ 1111_0000000000000000b       ;接下来将19～16位全部置为1
DESC_LIMIT_DATA2    equ DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2    equ 0000_000000000000000b

DESC_P      equ      1_000000000000000b
DESC_DPL_0  equ       00_0000000000000b
DESC_DPL_1  equ       01_0000000000000b
DESC_DPL_2  equ       10_0000000000000b
DESC_DPL_3  equ       11_0000000000000b

DESC_S_CODE equ         1_000000000000b
DESC_S_DATA equ       DESC_S_CODE
DESC_S_sys  equ         0_000000000000b

DESC_TYPE_CODE equ       1000_00000000b
;x=1,c=0,r=0,a=0 代码段是可执行的，非一致性，不可读，已访问位a清0
DESC_TYPE_DATA equ       0010_00000000b
;x=0,e=0,w=1,a=0 数据段是不可执行的，向上扩展的，可写,已访问位a清0 

;定义代码段，数据段，显存段的高32位
;注意下面在使用\进行换行时，\后面不允许跟任何的字符包括空格和注释符号;
DESC_CODE_HIGH4  equ    (0x00<<24)+ \
                        DESC_G_4K+DESC_D_32+DESC_L+DESC_AVL+DESC_LIMIT_CODE2+ \
                        DESC_P+DESC_DPL_0+DESC_S_CODE+DESC_TYPE_CODE + 0x00

DESC_DATA_HIGH4  equ    (0x00<<24)+ \
                        DESC_G_4K+DESC_D_32+DESC_L+DESC_AVL+DESC_LIMIT_DATA2+ \
                        DESC_P+DESC_DPL_0+DESC_S_DATA+DESC_TYPE_DATA + 0x00

DESC_VIDEO_HIGH4 equ    (0x00<<24)+ \
                        DESC_G_4K+DESC_D_32+DESC_L+DESC_AVL+DESC_LIMIT_VIDEO2+ \
                        DESC_P+DESC_DPL_0+DESC_S_DATA+DESC_TYPE_DATA + 0x0b
                        
;定义选择子RPL为对应的数字
RPL0 equ 00b
RPL1 equ 01b
RPL2 equ 10b
RPL3 equ 11b

TI_GDT equ 000b
TI_LDT equ 001b

;页表相关位的宏定义

;---------模块化的页目录表字段,PWT PCD A D G AVL 暂时不用设置   ----------
PG_P  equ   1b
PG_RW_R	 equ  00b 
PG_RW_W	 equ  10b 
PG_US_S	 equ  000b 
PG_US_U	 equ  100b  

LOADER_STACK_TOP equ LOADER_BASE_ADDR               

KERNEL_BIN_BASE_ADDR equ 0x70000                    ;定义内核在内存中的缓冲区，也就是将编译好的内核文件暂时存储在内存中的位置
KERNEL_START_SECTOR equ 0x9                         ;定义内核在磁盘的起始扇区
KERNEL_ENTRY_POINT equ 0xc0001500                   ;定义内核可执行代码的入口地址

PT_NULL equ 0                                       ;程序段的 type 定义
