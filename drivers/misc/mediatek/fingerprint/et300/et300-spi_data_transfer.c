#include <mach/mt_spi.h>
#include "et300_data_transfer.h"

#define SPI_FIFO_SIZE 32

#define SPI_TRANSFER_SPEED_HZ (12 * 1000 * 1000)


#define DMA_ENABLE 1
#define DMA_DISABLE 0

#ifdef TRANSFER_MODE_DMA
void mt_spi_dma_mode_setup(struct spi_device *spi, int enable)
{
	struct mt_chip_conf *chip_config = (struct mt_chip_conf *) spi->controller_data;
	chip_config->com_mod = (enable!=0?DMA_TRANSFER:FIFO_TRANSFER);
	spi_setup(spi);
}
// burst mode
int et300_io_mass_read(struct et300_data* et300, u8 addr, u8* buf, int read_len)
{
	int status = 0;
	struct spi_device *spi;
	struct spi_message m;
	u8 *write_addr = (u8*)kzalloc(2, GFP_KERNEL);
	write_addr[0] = ET300_WRITE_ADDRESS;
	write_addr[1] = addr;
	u8 *read_data = (u8 *) kzalloc(read_len + 1, GFP_KERNEL);
	//Write and read data in one data query section
	struct spi_transfer t_set_addr = {
		.tx_buf = write_addr,
		.len = 2,
		.speed_hz = SPI_TRANSFER_SPEED_HZ,
	};
	struct spi_transfer t_read_data = {
		.tx_buf = read_data,
		.rx_buf = read_data,
		.len = read_len,
		.speed_hz = SPI_TRANSFER_SPEED_HZ,
	};

	DEBUG_PRINT("%s read_len = %d", __func__, read_len);
	//printk(KERN_ERR "%s read_len = %d", __func__, read_len);

	read_data[0] = ET300_READ_DATA;

	spi = et300->spi;

	spi_message_init(&m);
	spi_message_add_tail(&t_set_addr, &m);
	spi_message_add_tail(&t_read_data, &m);
	status = spi_sync(spi, &m);
	memcpy(buf, read_data + 1, read_len);
	kfree(read_data);
	if(status < 0)
	{
		printk(KERN_ERR "%s read data error status = %d\n", __func__, status);
	}

	if(write_addr)
		kfree(write_addr);

	return status;

}
#else
int et300_io_mass_read(struct et300_data* et300, u8 addr, u8* buf, int read_len)
{
	int status = 0;
	struct spi_device *spi;
	struct spi_message m;

	//Set start address
	u8 write_addr[] = {ET300_WRITE_ADDRESS, addr};
	u8 read_opcode[]={ET300_READ_DATA,0xff};
	u8 read_result[]={0xff,0xff};
	//Write and read data in one data query section
	struct spi_transfer *t3 = kmalloc(sizeof(struct spi_transfer)*read_len,GFP_KERNEL);
	if(t3 == NULL)
		printk(KERN_ERR "-----------------------------------null");
	memset(t3, 0, sizeof(struct spi_transfer)*read_len);

	struct spi_transfer *t4 = kmalloc(sizeof(struct spi_transfer)*read_len,GFP_KERNEL);
	if(t4 == NULL)
		printk(KERN_ERR "-----------------------------------null");
	memset(t4, 0, sizeof(struct spi_transfer)*read_len);
	
	buf[0] = ET300_READ_DATA;

	spi = et300->spi;

	uint8_t *tmp;
	tmp=kmalloc(read_len*2, GFP_KERNEL);

	spi_message_init(&m);
	
	int ii;
	for (ii=0;ii<read_len;ii++)
	{
		t4[ii].tx_buf =  write_addr;
		t4[ii].len = 2;
		//t4[ii].speed_hz = SPI_TRANSFER_SPEED_HZ;
		t3[ii].tx_buf = read_opcode;
		t3[ii].rx_buf = &tmp[ii*2];
		t3[ii].len = 2;
		//t3[ii].speed_hz = SPI_TRANSFER_SPEED_HZ;
		spi_message_add_tail((t4+ii), &m);
		spi_message_add_tail((t3+ii), &m);

	}
	status = spi_sync(spi, &m);

	/*if (!access_ok(VERIFY_WRITE, (u8 __user *) (uintptr_t) buf, read_len))
	{
		status = -1;
		printk(KERN_ERR "buffer checking fail");
		goto out;
	}*/
	/*if (copy_from_user(kbuf, (const u8 __user *) (uintptr_t) buf, read_len))
	{
		printk(KERN_ERR "buffer copy_from_user fail status = %d", status);
		status = -EFAULT;
		goto out;
	}*/
	for (ii=0; ii<read_len; ii++)
		buf[ii] = tmp [ii*2 + 1];	
	/*if (copy_to_user((u8 __user *) (uintptr_t) buf, kbuf, read_len)) 
	{
		printk(KERN_ERR "buffer copy_to_user fail status = %d", status);
		status = -EFAULT;
		goto out;
	}*/
	kfree(tmp);
	kfree(t3);
	kfree(t4);
	if(status < 0){
		printk(KERN_ERR "%s read data error status = %d\n", __func__, status);
		goto out;
	}

out:
	return status;

}
#endif
/*
 * Read registers
 * addr include all registers' address will be read
 * structure of tx_buf is [ address1, address2 ...]
 * Read registers by a loop and read one register once
 */
int et300_io_bulk_read(struct et300_data* et300, u8* addr, u8* buf, int read_len)
{
	//printk(KERN_ERR "et300_io_bulk_read\n");
	int status = 0;
	struct spi_device *spi;
	struct spi_message m;
	int pos = 0;

	u8* addrval = kzalloc(read_len, GFP_KERNEL);
	if (copy_from_user(addrval, (const u8 __user *) (uintptr_t) addr, read_len))
	{
		printk(KERN_ERR "%s buffer copy_from_user fail", __func__);
		status = -EFAULT;
		goto out;
	}
	u8 *write_addr = (u8*)kzalloc(2,GFP_KERNEL);
	write_addr[0] = ET300_WRITE_ADDRESS;
	write_addr[1] = addrval[0];
	u8 *read_value = (u8*)kzalloc(2,GFP_KERNEL);
	read_value[0] = ET300_READ_DATA;
	read_value[1] = 0x00;

	u8* val = (u8*)kzalloc(read_len, GFP_KERNEL);

	u8 *result;
	result = (u8*)kzalloc(2, GFP_KERNEL);
	

	struct spi_transfer t_set_addr={
		.tx_buf= write_addr,
		.len = 2,
		//.speed_hz = SPI_TRANSFER_SPEED_HZ,
	};
	struct spi_transfer t = {
		.tx_buf = read_value,
		.rx_buf = result,
		.len = 2,
		//.speed_hz = SPI_TRANSFER_SPEED_HZ,
	};

	DEBUG_PRINT("%s read_len = %d", __func__, read_len);

	spi = et300->spi;

	pos = 0;
	do{
		//Set address
		//write_addr[1] = addr[pos];
		write_addr[1] = addrval[pos];
		//Start to read
		spi_message_init(&m);
		spi_message_add_tail(&t_set_addr, &m);
		spi_message_add_tail(&t, &m);
		status = spi_sync(spi, &m);
		if(status < 0)
		{
			printk(KERN_ERR "%s read data error status = %d\n", __func__, status);
			goto out;
		}
		//Copy result, result[0] don't care
		//buf[pos] = result[1];
		val[pos] = result[1];
		pos++;
	}
	while(pos < read_len);

#ifdef ET300_SPI_DEBUG
        int i;
	for(i=0;i<read_len;i++)
	{
		DEBUG_PRINT("%s address = %x buf = %x", __func__, addr[i], val[i]);
	}
#endif
  if (copy_to_user((u8 __user *) (uintptr_t) buf, val, read_len)) 
  {
	  //printk(KERN_ERR "buffer copy_to_user fail status = %d", status);
	  printk(KERN_ERR "%s buffer copy_to_user fail status", __func__);
	  status = -EFAULT;
	  goto out;
  }

out:
	if(addrval)
		kfree(addrval);
	if(val)
		kfree(val);
	if(write_addr)
		kfree(write_addr);
	if(read_value)
		kfree(read_value);
	if(result)
		kfree(result);
	return status;

}

/*
 * Write data to registers
 * tx_buf includes address and value will be wrote
 * structure of tx_buf is [{ address1, value1 }, {address2, value2} ...]
 */
int et300_io_bulk_write(struct et300_data* et300, u8* buf, int write_len)
{
	//printk(KERN_ERR "et300_io_bulk_write\n");
	int status = 0;
	struct spi_device *spi;
	int pos = 0;
	struct spi_message m;

	u8 *write_addr = (u8*)kzalloc(2,GFP_KERNEL);
	write_addr[0] = ET300_WRITE_ADDRESS;
	write_addr[1] = 0x00;
	u8 *write_value = (u8*)kzalloc(2,GFP_KERNEL);
	write_value[0] = ET300_WRITE_DATA;
	write_value[1] = 0x00;
	u8* val = (u8*)kzalloc(write_len, GFP_KERNEL);
	if (copy_from_user(val, (const u8 __user *) (uintptr_t) buf, write_len))
	{
		printk(KERN_ERR "%s buffer copy_from_user fail", __func__);
		status = -EFAULT;
		goto out;
	}

	struct spi_transfer t1={
		.tx_buf= write_addr,
		.len = 2,
		//.speed_hz = SPI_TRANSFER_SPEED_HZ,
	};
	struct spi_transfer t2 = {
		.tx_buf = write_value,
		.len = 2,
		//.speed_hz = SPI_TRANSFER_SPEED_HZ,
	};

	DEBUG_PRINT("%s write_len = %d", __func__, write_len);

	if((write_len % 2) != 0)
	{
		printk(KERN_ERR "%s invalid write buffer (length)", __func__);
		status = -EFAULT;
		goto out;
	}

#ifdef ES608_SPI_DEBUG
	for(i=0;i<write_len/2;i++)
	{
		//DEBUG_PRINT("%s address = %x data = %x", __func__, buf[i*2], buf[i*2+1]);
		DEBUG_PRINT("%s address = %x data = %x", __func__, val[i*2], val[i*2+1]);
	}
#endif

	spi = et300->spi;

	pos = 0;
	do{
		//Set address
		//write_addr[1] = buf[pos*2];
		write_addr[1] = val[pos*2];
		//Set value
		//write_value[1] = buf[pos*2+1];
		write_value[1] = val[pos*2+1];
		spi_message_init(&m);
		spi_message_add_tail(&t1, &m);
		spi_message_add_tail(&t2, &m);
		status = spi_sync(spi, &m);
		if(status < 0)
		{
			printk(KERN_ERR "%s read data error status = %d", __func__, status);
			goto out;
		}
		++pos;
	}
	while(pos < write_len/2);


out:
	if(write_addr)
		kfree(write_addr);
	if(write_value)
		kfree(write_value);
	if(val)
		kfree(val);
	return status;

}
int et300_read_register(struct et300_data* et300, u8 addr, u8* buf)
{
	int	status = 0;
	struct spi_device *spi;
	struct spi_message m;

	//Set address
	u8 *write_addr = (u8*)kzalloc(2, GFP_KERNEL);
	write_addr[0] = ET300_WRITE_ADDRESS;
	write_addr[1] = addr;
	u8 *read_value = (u8*)kzalloc(2, GFP_KERNEL);
	read_value[0] = ET300_READ_DATA;
	read_value[1] = 0x00;
	u8 *result = (u8*)kzalloc(2, GFP_KERNEL);
	result[0] = 0x00;
	result[1] = 0x00;

	struct spi_transfer t1 = {
		.tx_buf = write_addr,
		.len = 2,
		//.speed_hz = SPI_TRANSFER_SPEED_HZ,
	};
	struct spi_transfer t2 ={
		.tx_buf = read_value,
		.rx_buf	= result,
		.len = 2,	
		//.speed_hz = SPI_TRANSFER_SPEED_HZ,
	};

	spi = et300->spi;
	spi_message_init(&m);
	spi_message_add_tail(&t1, &m);
	spi_message_add_tail(&t2, &m);
	status = spi_sync(spi, &m);

	if(status < 0)
	{
		printk(KERN_ERR "%s read data error status = %d\n", __func__, status);
		goto out;
	}
	//Copy result
	*buf = result[1];

	DEBUG_PRINT("es608_read_register address = %x result = %x %x\n", addr, result[0], result[1]);

out:
	if(write_addr)
		kfree(write_addr);
	if(read_value)
		kfree(read_value);
	if(result)
		kfree(result);
	return status;
}
int et300_write_register(struct et300_data* et300, u8 addr, u8 value)
{
	int	status = 0;
	struct spi_device *spi;
	struct spi_message m;

	//Set address
	u8 *write_addr = (u8*)kzalloc(2, GFP_KERNEL);
	write_addr[0] = ET300_WRITE_ADDRESS;
	write_addr[1] = addr;
	//Set value
	u8 *write_value = (u8*)kzalloc(2, GFP_KERNEL);
	write_value[0] = ET300_WRITE_DATA;
	write_value[1] = value;
	struct spi_transfer t1={
		.tx_buf = write_addr,
		.len = 2,
	};
	struct spi_transfer t2={
		.tx_buf = write_value,
		.len = 2,
	};
	DEBUG_PRINT("es608_write_register address = %x value = %x\n", addr,value);
	spi = et300->spi;
	spi_message_init(&m);
	spi_message_add_tail(&t1, &m);
	spi_message_add_tail(&t2, &m);
	status = spi_sync(spi, &m);
	
	if(status < 0)
	{
		printk(KERN_ERR "%s spi_write error status = %d\n", __func__, status);
		goto out;
	}

	out:
	if(write_addr)
		kfree(write_addr);
	if(write_value)
		kfree(write_value);
	return status;
}

int et300_get_one_image(
	struct et300_data *et300, 
	u8 *buf, 
	u8 *image_buf
	)
{
  uint8_t read_val;
  int32_t status = 0;
  uint32_t fail_count = 0;	
  uint8_t *tx_buf = (uint8_t*)buf;
  uint8_t *rx_buf = (uint8_t*)image_buf;
  uint32_t read_count;
  //uint8_t work_buf[1024];

  uint8_t *work_buf=NULL;

  //
  // transfer data format
  // tx_buf[0] = image width
  // tx_buf[1] = image hight;
  // tx_buf[2] = 0x01;
  // tx_buf[3] = TBD;
  // tx_buf[4] = TBD;
  // tx_buf[5] = TBD;
  //
  //read_count = tx_buf[0] * tx_buf[1];          // total pixel , width * hight
  uint8_t *val = (uint8_t*) kzalloc(6, GFP_KERNEL);
  if (copy_from_user(val, (const u8 __user *) (uintptr_t) tx_buf, 6))
  {
	  printk(KERN_ERR "%s buffer copy_from_user fail", __func__);
	  status = -EFAULT;
	  goto done;
  }
  read_count = val[0] * val[1];          // total pixel , width * hight
  
  DEBUG_PRINT("%s, imge_count=%d\n", __func__,read_count); 


  while(1)
  {
    et300_read_register(et300, FSTATUS_ET300_ADDR, &read_val);
    if(read_val & FRM_RDY_ET300)
    {
      break;
    }
	if(fail_count >=250)
	{
		printk(KERN_ERR "%s: fail_count = %d\n",__func__, fail_count);
		status = -1;
		goto image_not_ready;
	}
	fail_count++;
  }

  work_buf = kzalloc(read_count, GFP_KERNEL);
  status = et300_io_mass_read(et300, FDATA_ET300_ADDR, work_buf, read_count);
  if(status < 0)
  {
    printk(KERN_ERR "%s call et300_io_mass_read error status = %d", __func__, status);
      goto done;
  }



done:
  //memcpy(image_buf, work_buf, read_count); 
  if (copy_to_user((u8 __user *) (uintptr_t) image_buf, work_buf, read_count)) 
  {
	  printk(KERN_ERR "buffer copy_to_user fail status = %d", status);
	  status = -EFAULT;
	  //goto out;
  }
image_not_ready:
  if(work_buf)
	kfree(work_buf);
  if(val)
	kfree(val);
  return status;
}



