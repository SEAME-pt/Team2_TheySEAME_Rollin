#ifndef IIS2MDC_COMPAT_H
#define IIS2MDC_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "iis2mdc_reg.h"

#define IIS2MDC_OK             0
#define IIS2MDC_ERROR         -1

#define IIS2MDC_I2C_BUS        0U
#define IIS2MDC_SPI_4WIRES_BUS 1U
#define IIS2MDC_SPI_3WIRES_BUS 2U

typedef int32_t (*IIS2MDC_Init_Func)(void);
typedef int32_t (*IIS2MDC_DeInit_Func)(void);
typedef int32_t (*IIS2MDC_GetTick_Func)(void);
typedef int32_t (*IIS2MDC_WriteReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*IIS2MDC_ReadReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);

typedef struct
{
  IIS2MDC_WriteReg_Func  WriteReg;
  IIS2MDC_ReadReg_Func   ReadReg;
  IIS2MDC_Init_Func      Init;
  IIS2MDC_DeInit_Func    DeInit;
  IIS2MDC_GetTick_Func   GetTick;
  uint8_t                BusType;
  uint16_t               Address;
} IIS2MDC_IO_t;

typedef struct
{
  stmdev_ctx_t Ctx;
  IIS2MDC_IO_t IO;
  uint8_t is_initialized;
  uint8_t mag_is_enabled;
} IIS2MDC_Object_t;

typedef struct
{
  int32_t (*Init)(IIS2MDC_Object_t *);
  int32_t (*DeInit)(IIS2MDC_Object_t *);
  int32_t (*ReadID)(IIS2MDC_Object_t *, uint8_t *);
  int32_t (*GetCapabilities)(IIS2MDC_Object_t *, void *);
} IIS2MDC_CommonDrv_t;

typedef struct
{
  int32_t (*Enable)(IIS2MDC_Object_t *);
  int32_t (*Disable)(IIS2MDC_Object_t *);
  int32_t (*GetSensitivity)(IIS2MDC_Object_t *, float *);
  int32_t (*GetOutputDataRate)(IIS2MDC_Object_t *, float *);
  int32_t (*SetOutputDataRate)(IIS2MDC_Object_t *, float);
  int32_t (*GetFullScale)(IIS2MDC_Object_t *, int32_t *);
  int32_t (*SetFullScale)(IIS2MDC_Object_t *, int32_t);
  int32_t (*GetAxes)(IIS2MDC_Object_t *, void *);
  int32_t (*GetAxesRaw)(IIS2MDC_Object_t *, void *);
} IIS2MDC_MagDrv_t;

typedef struct
{
  uint8_t  Acc;
  uint8_t  Gyro;
  uint8_t  Magneto;
  uint8_t  LowPower;
  uint32_t GyroMaxFS;
  uint32_t AccMaxFS;
  uint32_t MagMaxFS;
  float    GyroMaxOdr;
  float    AccMaxOdr;
  float    MagMaxOdr;
} IIS2MDC_Capabilities_t;

typedef struct
{
  int32_t x;
  int32_t y;
  int32_t z;
} IIS2MDC_Axes_t;

typedef struct
{
  int16_t x;
  int16_t y;
  int16_t z;
} IIS2MDC_AxesRaw_t;

static int32_t IIS2MDC_ReadRegWrap(void *handle, uint8_t reg, uint8_t *data, uint16_t len)
{
  IIS2MDC_Object_t *obj = (IIS2MDC_Object_t *)handle;

  if ((obj == 0) || (obj->IO.ReadReg == 0))
  {
    return IIS2MDC_ERROR;
  }

  return obj->IO.ReadReg(obj->IO.Address, reg, data, len);
}

static int32_t IIS2MDC_WriteRegWrap(void *handle, uint8_t reg, const uint8_t *data, uint16_t len)
{
  IIS2MDC_Object_t *obj = (IIS2MDC_Object_t *)handle;

  if ((obj == 0) || (obj->IO.WriteReg == 0))
  {
    return IIS2MDC_ERROR;
  }

  return obj->IO.WriteReg(obj->IO.Address, reg, (uint8_t *)data, len);
}

static int32_t IIS2MDC_Init(IIS2MDC_Object_t *obj)
{
  if (obj == 0)
  {
    return IIS2MDC_ERROR;
  }

  if ((obj->IO.Init != 0) && (obj->IO.Init() != IIS2MDC_OK))
  {
    return IIS2MDC_ERROR;
  }

  if (iis2mdc_block_data_update_set(&(obj->Ctx), PROPERTY_ENABLE) != 0)
  {
    return IIS2MDC_ERROR;
  }

  if (iis2mdc_data_rate_set(&(obj->Ctx), IIS2MDC_ODR_10Hz) != 0)
  {
    return IIS2MDC_ERROR;
  }

  if (iis2mdc_operating_mode_set(&(obj->Ctx), IIS2MDC_CONTINUOUS_MODE) != 0)
  {
    return IIS2MDC_ERROR;
  }

  obj->is_initialized = 1U;
  obj->mag_is_enabled = 1U;

  return IIS2MDC_OK;
}

static int32_t IIS2MDC_DeInit(IIS2MDC_Object_t *obj)
{
  if (obj == 0)
  {
    return IIS2MDC_ERROR;
  }

  (void)iis2mdc_operating_mode_set(&(obj->Ctx), IIS2MDC_POWER_DOWN);
  obj->mag_is_enabled = 0U;
  obj->is_initialized = 0U;

  if ((obj->IO.DeInit != 0) && (obj->IO.DeInit() != IIS2MDC_OK))
  {
    return IIS2MDC_ERROR;
  }

  return IIS2MDC_OK;
}

static int32_t IIS2MDC_ReadID(IIS2MDC_Object_t *obj, uint8_t *id)
{
  if ((obj == 0) || (id == 0))
  {
    return IIS2MDC_ERROR;
  }

  return (iis2mdc_device_id_get(&(obj->Ctx), id) == 0) ? IIS2MDC_OK : IIS2MDC_ERROR;
}

static int32_t IIS2MDC_GetCapabilities(IIS2MDC_Object_t *obj, void *capabilities)
{
  IIS2MDC_Capabilities_t *caps = (IIS2MDC_Capabilities_t *)capabilities;

  if ((obj == 0) || (caps == 0))
  {
    return IIS2MDC_ERROR;
  }

  caps->Acc = 0U;
  caps->Gyro = 0U;
  caps->Magneto = 1U;
  caps->LowPower = 1U;
  caps->GyroMaxFS = 0U;
  caps->AccMaxFS = 0U;
  caps->MagMaxFS = 50U;
  caps->GyroMaxOdr = 0.0f;
  caps->AccMaxOdr = 0.0f;
  caps->MagMaxOdr = 100.0f;

  return IIS2MDC_OK;
}

static int32_t IIS2MDC_Enable(IIS2MDC_Object_t *obj)
{
  if (obj == 0)
  {
    return IIS2MDC_ERROR;
  }

  if (iis2mdc_operating_mode_set(&(obj->Ctx), IIS2MDC_CONTINUOUS_MODE) != 0)
  {
    return IIS2MDC_ERROR;
  }

  obj->mag_is_enabled = 1U;
  return IIS2MDC_OK;
}

static int32_t IIS2MDC_Disable(IIS2MDC_Object_t *obj)
{
  if (obj == 0)
  {
    return IIS2MDC_ERROR;
  }

  if (iis2mdc_operating_mode_set(&(obj->Ctx), IIS2MDC_POWER_DOWN) != 0)
  {
    return IIS2MDC_ERROR;
  }

  obj->mag_is_enabled = 0U;
  return IIS2MDC_OK;
}

static int32_t IIS2MDC_GetSensitivity(IIS2MDC_Object_t *obj, float *sensitivity)
{
  if ((obj == 0) || (sensitivity == 0))
  {
    return IIS2MDC_ERROR;
  }

  *sensitivity = 1.5f;
  return IIS2MDC_OK;
}

static int32_t IIS2MDC_GetOutputDataRate(IIS2MDC_Object_t *obj, float *odr)
{
  iis2mdc_odr_t reg_odr;

  if ((obj == 0) || (odr == 0))
  {
    return IIS2MDC_ERROR;
  }

  if (iis2mdc_data_rate_get(&(obj->Ctx), &reg_odr) != 0)
  {
    return IIS2MDC_ERROR;
  }

  switch (reg_odr)
  {
    case IIS2MDC_ODR_10Hz:
      *odr = 10.0f;
      break;
    case IIS2MDC_ODR_20Hz:
      *odr = 20.0f;
      break;
    case IIS2MDC_ODR_50Hz:
      *odr = 50.0f;
      break;
    case IIS2MDC_ODR_100Hz:
      *odr = 100.0f;
      break;
    default:
      return IIS2MDC_ERROR;
  }

  return IIS2MDC_OK;
}

static int32_t IIS2MDC_SetOutputDataRate(IIS2MDC_Object_t *obj, float odr)
{
  iis2mdc_odr_t reg_odr;

  if (obj == 0)
  {
    return IIS2MDC_ERROR;
  }

  if (odr <= 10.0f)
  {
    reg_odr = IIS2MDC_ODR_10Hz;
  }
  else if (odr <= 20.0f)
  {
    reg_odr = IIS2MDC_ODR_20Hz;
  }
  else if (odr <= 50.0f)
  {
    reg_odr = IIS2MDC_ODR_50Hz;
  }
  else
  {
    reg_odr = IIS2MDC_ODR_100Hz;
  }

  return (iis2mdc_data_rate_set(&(obj->Ctx), reg_odr) == 0) ? IIS2MDC_OK : IIS2MDC_ERROR;
}

static int32_t IIS2MDC_GetFullScale(IIS2MDC_Object_t *obj, int32_t *full_scale)
{
  if ((obj == 0) || (full_scale == 0))
  {
    return IIS2MDC_ERROR;
  }

  *full_scale = 50;
  return IIS2MDC_OK;
}

static int32_t IIS2MDC_SetFullScale(IIS2MDC_Object_t *obj, int32_t full_scale)
{
  (void)obj;
  (void)full_scale;
  return IIS2MDC_ERROR;
}

static int32_t IIS2MDC_GetAxesRaw(IIS2MDC_Object_t *obj, void *axes_raw)
{
  int16_t raw[3];
  IIS2MDC_AxesRaw_t *axes = (IIS2MDC_AxesRaw_t *)axes_raw;

  if ((obj == 0) || (axes == 0))
  {
    return IIS2MDC_ERROR;
  }

  if (iis2mdc_magnetic_raw_get(&(obj->Ctx), raw) != 0)
  {
    return IIS2MDC_ERROR;
  }

  axes->x = raw[0];
  axes->y = raw[1];
  axes->z = raw[2];

  return IIS2MDC_OK;
}

static int32_t IIS2MDC_GetAxes(IIS2MDC_Object_t *obj, void *axes_out)
{
  int16_t raw[3];
  IIS2MDC_Axes_t *axes = (IIS2MDC_Axes_t *)axes_out;

  if ((obj == 0) || (axes == 0))
  {
    return IIS2MDC_ERROR;
  }

  if (iis2mdc_magnetic_raw_get(&(obj->Ctx), raw) != 0)
  {
    return IIS2MDC_ERROR;
  }

  axes->x = ((int32_t)raw[0] * 15) / 10;
  axes->y = ((int32_t)raw[1] * 15) / 10;
  axes->z = ((int32_t)raw[2] * 15) / 10;

  return IIS2MDC_OK;
}

static int32_t IIS2MDC_RegisterBusIO(IIS2MDC_Object_t *obj, IIS2MDC_IO_t *io)
{
  if ((obj == 0) || (io == 0))
  {
    return IIS2MDC_ERROR;
  }

  obj->IO = *io;
  obj->Ctx.read_reg = IIS2MDC_ReadRegWrap;
  obj->Ctx.write_reg = IIS2MDC_WriteRegWrap;
  obj->Ctx.handle = obj;
  obj->Ctx.mdelay = 0;

  return IIS2MDC_OK;
}

static IIS2MDC_CommonDrv_t IIS2MDC_COMMON_Driver =
{
  IIS2MDC_Init,
  IIS2MDC_DeInit,
  IIS2MDC_ReadID,
  IIS2MDC_GetCapabilities
};

static IIS2MDC_MagDrv_t IIS2MDC_MAG_Driver =
{
  IIS2MDC_Enable,
  IIS2MDC_Disable,
  IIS2MDC_GetSensitivity,
  IIS2MDC_GetOutputDataRate,
  IIS2MDC_SetOutputDataRate,
  IIS2MDC_GetFullScale,
  IIS2MDC_SetFullScale,
  IIS2MDC_GetAxes,
  IIS2MDC_GetAxesRaw
};

#ifdef __cplusplus
}
#endif

#endif /* IIS2MDC_COMPAT_H */