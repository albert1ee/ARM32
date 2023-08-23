
#include "main.h"
#include "sync_upload.h"
#include "clifeapp.h"
#include "motor_action_track_app.h"

struct _ble_notify
{
	uint32_t motor_press_notify_ticks;
	
	uint32_t mpnt;

	//电机同步上传数据类型
	uint8_t motor_data_upload_type;
};

struct _ble_notify blenoifytick=
{
	.motor_press_notify_ticks = 100,
	.mpnt = 0,
	.motor_data_upload_type = 1,
};

void sync_upload_set_motor_ticks(uint32_t tick)
{
	blenoifytick.motor_press_notify_ticks = tick;
	blenoifytick.mpnt = rt_tick_get();
}
uint32_t sync_upload_get_motor_ticks(void)
{
	return blenoifytick.motor_press_notify_ticks;
}
void sync_upload_set_motor_upload_type(uint8_t type)
{
	blenoifytick.motor_data_upload_type = type;
}
uint8_t sync_upload_get_motor_upload_type(void)
{
	return blenoifytick.motor_data_upload_type;
}
//电机同步上传
void sync_upload_motor_data_upload(int parent,int type)
{
	//获取模式与电机信息
	uint32_t marksize = 0;
	uint32_t size = 2;
	uint8_t * pbuf = 0;
	motorpoint_t * pmp = getmotorpointinfo();
	int mode = (eMotorControlMode)mat_get_mode();
	if (type == -1)
		type = blenoifytick.motor_data_upload_type;
	if ((mode != eNullMode) && (mode != eFreeMode))
	{
		size+=4;//file id
		size++;	//type
		//返回占空比和频率
		if (type == 0x00)
		{
			size += pmp->motors * 2;
		}
		//返回占空比
		else if (type == 0x01)
		{
			size += pmp->motors * 1;
		}
		//返回频率
		else if (type == 0x02)
		{
			size += pmp->motors * 1;
		}
		marksize = ((pmp->motors >> 3) + (pmp->motors & 0x07 ? 1 : 0));
		size += marksize;
		size += 2; //len
	}
	pbuf = rt_malloc(size);
	if (pbuf != 0)
	{
		memset(pbuf,0,size);
		if ((mode != eNullMode) && (mode != eFreeMode))
		{
			int offset = 2;
			int id = mat_get_fileid_by_index(mat_get_index());
			set_bytes_big_end_from_data(pbuf+offset,id,4);
			offset += 4;
			pbuf[offset++] = type;
			
			size = 0;
			//返回占空比和频率
			if (type == 0x00)
			{
				for(int i=0;i<pmp->motors;i++)
				{
					pbuf[offset + marksize + 2 + 2 * i + 0] = serial_motor_get_freq(i);
					pbuf[offset + marksize + 2 + 2 * i + 1] = serial_motor_get_duty(i);
				}
				MotorCompress(pbuf + offset + marksize + 2,pmp->motors,pbuf + offset,&size);
			}
			//返回占空比
			else if (type == 0x01)
			{
				for(int i=0;i<pmp->motors;i++)
				{
					pbuf[offset + marksize + 2 + i] = serial_motor_get_duty(i);
				}
				U8Compress(pbuf + offset + marksize + 2,pmp->motors,pbuf + offset,&size);
			}
			//返回频率
			else if (type == 0x02)
			{
				for(int i=0;i<pmp->motors;i++)
				{
					pbuf[offset + marksize + 2 + i] = serial_motor_get_freq(i);
				}
				U8Compress(pbuf + offset + marksize + 2,pmp->motors,pbuf + offset,&size);
			}
			size += offset;//mode & type
		}
		mode *= 100;
		if (mode == 100)
		{
			mode += mat_get_index();
		}
		pbuf[0] = mode >> 8;
		pbuf[1] = mode & 0xFF;
		clife_add_frame(-1,0,parent,0x00,size,pbuf);
		rt_free(pbuf);
	}
}

void sync_upload_proc(void)
{
	if ((blenoifytick.motor_press_notify_ticks) && ((rt_tick_get() - (blenoifytick.mpnt + blenoifytick.motor_press_notify_ticks)) <= RT_TICK_MAX / 2))
	{
		blenoifytick.mpnt = rt_tick_get();
		if (clife_app_is_cert())
		{
			sync_upload_motor_data_upload(PI_NOTIFY,-1);
		}
	}
}

