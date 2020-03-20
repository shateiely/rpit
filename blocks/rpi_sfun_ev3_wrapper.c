/*
  *
  *   --- THIS FILE GENERATED BY S-FUNCTION BUILDER: 3.0 ---
  *
  *   This file is a wrapper S-function produced by the S-Function
  *   Builder which only recognizes certain fields.  Changes made
  *   outside these fields will be lost the next time the block is
  *   used to load, edit, and resave this file. This file will be overwritten
  *   by the S-function Builder block. If you want to edit this file by hand, 
  *   you must change it only in the area defined as:  
  *
  *        %%%-SFUNWIZ_wrapper_XXXXX_Changes_BEGIN 
  *            Your Changes go here
  *        %%%-SFUNWIZ_wrapper_XXXXXX_Changes_END
  *
  *   For better compatibility with the Real-Time Workshop, the
  *   "wrapper" S-function technique is used.  This is discussed
  *   in the Real-Time Workshop User's Manual in the Chapter titled,
  *   "Wrapper S-functions".
  *
  *   Created: Tue Oct  8 17:59:28 2013
  */


/*
 * Include Files
 *
 */
#if defined(MATLAB_MEX_FILE)
#include "tmwtypes.h"
#include "simstruc_types.h"
#else
#include "rtwtypes.h"
#endif

/* %%%-SFUNWIZ_wrapper_includes_Changes_BEGIN --- EDIT HERE TO _END */
#include <math.h>

#ifndef MATLAB_MEX_FILE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <libusb-1.0/libusb.h>
#include "bytecodes.h"
#endif
/* %%%-SFUNWIZ_wrapper_includes_Changes_END --- EDIT HERE TO _BEGIN */
#define u_width 1
#define y_width 1
/*
 * Create external references here.  
 *
 */
/* %%%-SFUNWIZ_wrapper_externs_Changes_BEGIN --- EDIT HERE TO _END */
#ifndef MATLAB_MEX_FILE

#define EV3_USB_TIMEOUT             1000    // Milliseconds
#define EV3_INTERFACE_NUMBER        0
#define EV3_CONFIGURATION_NB        1
#define EV3_EP_OUT                  0x01
#define EV3_EP_IN                   0x81
#define EV3_PACKET_SIZE							0x400
#define EV3_RESET_RESPONSE_SIZE     5
#define EV3_POWER_RESPONSE_SIZE     5
#define EV3_PROCESS_RESPONSE_SIZE   37
#define EV3_VENDOR_LEGO             0x0694
#define EV3_PRODUCT_EV3             0x0005
#define EV3_BYTECODE_ERROR_OFFSET   4
#define EV3_BYTECODE_OK             0x02
#define EV3_NB_MOTORS               4
#define EV3_NB_SENSORS              4
#define EV3_MOTOR_MAX_POWER         100
#define EV3_POWER_A_OFFSET          51
#define EV3_POWER_B_OFFSET          56
#define EV3_POWER_C_OFFSET          61
#define EV3_POWER_D_OFFSET          66
#define EV3_SENSORS_OFFSET          5
#define EV3_MAX_RETRY               3
#define EV3_RETRY_TIMEOUT           200000

/* Error codes */
#define EV3_OK                      0
#define EV3_USB_ERROR								1
#define EV3_NOT_PRESENT             2
#define EV3_CONFIGURATION_ERROR     3
#define EV3_IN_USE                  4
#define EV3_USB_WRITE_ERROR         5
#define EV3_USB_READ_ERROR          6
#define EV3_USB_PARTIAL_TRANS				7
#define EV3_USB_OVERFLOW						9
#define EV3_BYTECODE_ERROR          10


struct libusb_device_handle				  *ev3_hdl = NULL;

typedef int                         ev3_error_t;


ev3_error_t ev3_init( void )  {
	int status;
  
  status = libusb_init( NULL );
  
	if ( status ) 
		return EV3_USB_ERROR;

  return EV3_OK;
}


ev3_error_t ev3_find_and_open( void )  {
	libusb_device 									*dev, **devs;
	struct libusb_device_descriptor	desc;
	int															i, status, transfered;
  unsigned char	                  tmpbuf[EV3_PACKET_SIZE];

	if ( libusb_get_device_list( NULL, &devs ) < 0 )
		return EV3_USB_ERROR;
	
	/* Go through device list loooking for an EV3 device */
	for ( i = 0; ( dev = devs[i] ) != NULL; i++ ) {
		
		status = libusb_get_device_descriptor( dev, &desc );
		
		if ( status >= 0 ) {
			if ( 	( desc.idVendor == EV3_VENDOR_LEGO ) &&
						( desc.idProduct == EV3_PRODUCT_EV3 ) )	{
				
				/* Open the device */
				status = libusb_open( dev, &ev3_hdl );
				if ( status < 0 )	{
					libusb_free_device_list( devs, 1 );
					return EV3_USB_ERROR;
				}
				
				/* Detach possible kernel driver bound to interface */
  			libusb_detach_kernel_driver( ev3_hdl, EV3_INTERFACE_NUMBER );
  			
  			/* Claiming the interface */
				status = libusb_claim_interface( ev3_hdl, EV3_INTERFACE_NUMBER );
				if ( status )	{
					libusb_close( ev3_hdl );
					libusb_free_device_list( devs, 1 );
					return EV3_IN_USE;
				}
				
        /* Request a packet until getting a zero byte packet */
        do
        {
          status = libusb_bulk_transfer( ev3_hdl, EV3_EP_IN, tmpbuf, EV3_PACKET_SIZE, &transfered, EV3_USB_TIMEOUT );
        } while ( ( status == 0 ) && ( transfered != 0 ) ); 
        
				libusb_free_device_list( devs, 1 );
        return EV3_OK;
      }
		}
	}
	
	libusb_free_device_list( devs, 1 );
  return EV3_NOT_PRESENT;
}

ev3_error_t ev3_close( void )  {
  
  if ( ev3_hdl == NULL )
    return EV3_CONFIGURATION_ERROR;
  
  libusb_release_interface( ev3_hdl, EV3_INTERFACE_NUMBER );
  libusb_close( ev3_hdl );
  ev3_hdl = NULL;
  libusb_exit( NULL );

  return EV3_OK;
}

ev3_error_t ev3_send_buf( unsigned char *buf, int len )  {
	int	status, transfered = 0;
  
  if ( ev3_hdl == NULL )
    return EV3_CONFIGURATION_ERROR;
  
  status = libusb_bulk_transfer( ev3_hdl, EV3_EP_OUT, buf, len, &transfered, EV3_USB_TIMEOUT );
  
  if ( status  )
    return EV3_USB_WRITE_ERROR;
  
  if ( transfered != len )
  	return EV3_USB_PARTIAL_TRANS;

  return EV3_OK;
}

ev3_error_t ev3_recv_buf( unsigned char *buf, int len )  {
  int						i, status, transfered = 0;
  unsigned char	tmpbuf[EV3_PACKET_SIZE];
  
  if ( ev3_hdl == NULL )
    return EV3_CONFIGURATION_ERROR;
  
  if ( len > EV3_PACKET_SIZE )
  	return EV3_USB_OVERFLOW;
  
  status = libusb_bulk_transfer( ev3_hdl, EV3_EP_IN, tmpbuf, EV3_PACKET_SIZE, &transfered, EV3_USB_TIMEOUT );

  if ( status  )
    return EV3_USB_WRITE_ERROR;
  
  if ( transfered != EV3_PACKET_SIZE )
  	return EV3_USB_PARTIAL_TRANS;
  
  for ( i = 0; i < len; i++ )
  	buf[i] = tmpbuf[i];

  return EV3_OK;
}

/*
 *  Reset : stop all motors and clear counters
 */
ev3_error_t ev3_reset( void ) {
  int             status;
  unsigned char   ev3_bytecode[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, opOUTPUT_STOP, LC0(0), LC0(0x01), LC0(0), opOUTPUT_STOP, LC0(0), LC0(0x02), LC0(0), opOUTPUT_STOP, LC0(0), LC0(0x04), LC0(0), opOUTPUT_STOP, LC0(0), LC0(0x08), LC0(0), opOUTPUT_CLR_COUNT, LC0(0), LC0(0x01), opOUTPUT_CLR_COUNT, LC0(0), LC0(0x02), opOUTPUT_CLR_COUNT, LC0(0), LC0(0x04), opOUTPUT_CLR_COUNT, LC0(0), LC0(0x08) };
  unsigned char   ev3_response[EV3_RESET_RESPONSE_SIZE];
  
  /* Define the header of the bytecodes command */
  
  ev3_bytecode[0] = sizeof( ev3_bytecode ) - 2;
  
  /* Fill the response with zeros */
  
  memset( (void*)ev3_response, 0x00, sizeof( ev3_response ) );
  
  /* Send bytecodes to EV3 */
  
  status = ev3_send_buf( ev3_bytecode, sizeof( ev3_bytecode ) );
  if( status )
    return status;
  
  /* Get EV3 response */
  
  status = ev3_recv_buf( ev3_response, sizeof( ev3_response ) );
  if( status )
    return status;
  
  /* Check the length of the response */
  
  if ( ev3_response[0] != EV3_RESET_RESPONSE_SIZE - 2 )
    return EV3_BYTECODE_ERROR;
  
  /* Check the error status */
  
  if ( ev3_response[EV3_BYTECODE_ERROR_OFFSET] != EV3_BYTECODE_OK )
    return EV3_BYTECODE_ERROR;
  
  return EV3_OK;
}

/*
 *  Power : enable power for all motors
 */
ev3_error_t ev3_power( void ) {
  int             status;
  unsigned char   ev3_bytecode[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, opOUTPUT_POWER, LC0(0), LC0(0x01), LC1(0), opOUTPUT_START, LC0(0), LC0(0x01), opOUTPUT_POWER, LC0(0), LC0(0x02), LC1(0), opOUTPUT_START, LC0(0), LC0(0x02), opOUTPUT_POWER, LC0(0), LC0(0x04), LC1(0), opOUTPUT_START, LC0(0), LC0(0x04), opOUTPUT_POWER, LC0(0), LC0(0x08), LC1(0), opOUTPUT_START, LC0(0), LC0(0x08) };
  unsigned char   ev3_response[EV3_POWER_RESPONSE_SIZE];
  
  /* Define the header of the bytecodes command */
  
  ev3_bytecode[0] = sizeof( ev3_bytecode ) - 2;
  
  /* Fill the response with zeros */
  
  memset( (void*)ev3_response, 0x00, sizeof( ev3_response ) );
  
  /* Send bytecodes to EV3 */
  
  status = ev3_send_buf( ev3_bytecode, sizeof( ev3_bytecode ) );
  if( status )
    return status;
  
  /* Get EV3 response */
  
  status = ev3_recv_buf( ev3_response, sizeof( ev3_response ) );
  if( status )
    return status;
  
  /* Check the length of the response */
  
  if ( ev3_response[0] != EV3_RESET_RESPONSE_SIZE - 2 )
    return EV3_BYTECODE_ERROR;
  
  /* Check the error status */
  
  if ( ev3_response[EV3_BYTECODE_ERROR_OFFSET] != EV3_BYTECODE_OK )
    return EV3_BYTECODE_ERROR;
  
  return EV3_OK;
}

/*
 *  Process : send motor control signals and read all inputs
 */
ev3_error_t ev3_process( real_T *U, real_T *Y ) {
  int             status, i;
  signed char     power[EV3_NB_MOTORS];
  unsigned char   *powerpt;
  int             power_offsets[] = { EV3_POWER_A_OFFSET, EV3_POWER_B_OFFSET, EV3_POWER_C_OFFSET, EV3_POWER_D_OFFSET };
  int             portval;
  unsigned char   *portvalpt;
  unsigned char   ev3_bytecode[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, opINPUT_DEVICE, LC0(GET_RAW), LC0(0), LC0(16), GV0(0), opINPUT_DEVICE, LC0(GET_RAW), LC0(0), LC0(17), GV0(4), opINPUT_DEVICE, LC0(GET_RAW), LC0(0), LC0(18), GV0(8), opINPUT_DEVICE, LC0(GET_RAW), LC0(0), LC0(19), GV0(12), opINPUT_DEVICE, LC0(GET_RAW), LC0(0), LC0(0), GV0(16), opINPUT_DEVICE, LC0(GET_RAW), LC0(0), LC0(1), GV0(20), opINPUT_DEVICE, LC0(GET_RAW), LC0(0), LC0(2), GV0(24), opINPUT_DEVICE, LC0(GET_RAW), LC0(0), LC0(3), GV0(28), opOUTPUT_POWER, LC0(0), LC0(0x01), LC1(0), opOUTPUT_POWER, LC0(0), LC0(0x02), LC1(0), opOUTPUT_POWER, LC0(0), LC0(0x04), LC1(0), opOUTPUT_POWER, LC0(0), LC0(0x08), LC1(0) };
  unsigned char   ev3_response[EV3_PROCESS_RESPONSE_SIZE];
  
  /* Define the header of the bytecodes command */
  
  ev3_bytecode[0] = sizeof( ev3_bytecode ) - 2;
  
  /* Define motor power in the bytecodes command */
  
  for( i = 0; i < EV3_NB_MOTORS; i++ )  {
    if ( isnan( U[i] ) )  {
      power[i] = 0;
      continue;
    }
    if ( U[i] > EV3_MOTOR_MAX_POWER ) {
      power[i] = EV3_MOTOR_MAX_POWER;
      continue;
    }
    if ( U[i] < -EV3_MOTOR_MAX_POWER ) {
      power[i] = -EV3_MOTOR_MAX_POWER;
      continue;
    }
    power[i] = (signed char)round( U[i] );
  }
  
  powerpt = (unsigned char*)power;
  for( i = 0; i < EV3_NB_MOTORS; i++ )
    ev3_bytecode[power_offsets[i]] = powerpt[i];
  
  /* Fill the response with zeros */
  
  memset( (void*)ev3_response, 0x00, sizeof( ev3_response ) );
  
  /* Send bytecodes to EV3 */
  
  status = ev3_send_buf( ev3_bytecode, sizeof( ev3_bytecode ) );
  if( status )
    return status;
  
  /* Get EV3 response */
  
  status = ev3_recv_buf( ev3_response, sizeof( ev3_response ) );
  if( status )
    return status;
  
  /* Check the length of the response */
  
  if ( ev3_response[0] != EV3_PROCESS_RESPONSE_SIZE - 2 )
    return EV3_BYTECODE_ERROR;
  
  /* Check the error status */
  
  if ( ev3_response[EV3_BYTECODE_ERROR_OFFSET] != EV3_BYTECODE_OK )
    return EV3_BYTECODE_ERROR;
  
  /* Extract the sensor raw values from the response */
  
  portvalpt = (unsigned char*)&portval;
  for ( i = 0; i < EV3_NB_MOTORS + EV3_NB_SENSORS; i++ )  {
    for ( j = 0; j < sizeof( int ); j++ ) {
      portvalpt[j] = ev3_response[ EV3_SENSORS_OFFSET + i * sizeof( int ) + j ];
    }
    Y[i] = (real_T)portval;
  }
  
  return EV3_OK;
}

/*
 *  Start : initialize USB transactions with EV3
 */
ev3_error_t ev3_start( void ) {
  int status;
  
  status = ev3_init( );
  if( status )
    return status;
  
  status = ev3_find_and_open( );
  if( status )
    return status;
  
  ev3_reset( );
  
  status = ev3_power( );
  if( status )
    return status;
  
  return EV3_OK;
}

/*
 *  Terminate : shutdown USB connection
 */
ev3_error_t ev3_terminate( void ) {
  int status, i;
  
  for ( i = 0; i < EV3_MAX_RETRY; i++ ) {
    status = ev3_reset( );
    if ( status )
      usleep( EV3_RETRY_TIMEOUT  );
    else
      break;
  }
  
  ev3_close( );
  
  return status;
}

#endif
/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Output functions
 *
 */
void rpi_sfun_ev3_Outputs_wrapper(const real_T *u_A,
                          const real_T *u_B,
                          const real_T *u_C,
                          const real_T *u_D,
                          real_T *y_A,
                          real_T *y_B,
                          real_T *y_C,
                          real_T *y_D,
                          real_T *y_1,
                          real_T *y_2,
                          real_T *y_3,
                          real_T *y_4, 
                           const real_T  *rpi_Ts, const int_T p_width0)
{
/* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
/* Ouput calculation */
  
#ifdef MATLAB_MEX_FILE

y_A[0] = 0.0;
y_B[0] = 0.0;
y_C[0] = 0.0;
y_D[0] = 0.0;
y_1[0] = 0.0;
y_2[0] = 0.0;
y_3[0] = 0.0;
y_4[0] = 0.0;

#else

real_T  U[EV3_NB_MOTORS], Y[EV3_NB_MOTORS+EV3_NB_SENSORS];
int     status;

if ( p_width0 > 1 )	{
  fprintf( stderr, "** Multiple rates not allowed in this block **\n" );
  y_A[0] = 0.0;
  y_B[0] = 0.0;
  y_C[0] = 0.0;
  y_D[0] = 0.0;
  y_1[0] = 0.0;
  y_2[0] = 0.0;
  y_3[0] = 0.0;
  y_4[0] = 0.0;
  return;
}

if ( *rpi_Ts < 0.01 )	{
  fprintf( stderr, "** Max sampling rate = 100Hz **\n" );
  y_A[0] = 0.0;
  y_B[0] = 0.0;
  y_C[0] = 0.0;
  y_D[0] = 0.0;
  y_1[0] = 0.0;
  y_2[0] = 0.0;
  y_3[0] = 0.0;
  y_4[0] = 0.0;
  return;
}

U[0] = u_A[0];
U[1] = u_B[0];
U[2] = u_C[0];
U[3] = u_D[0];

status = ev3_process( U, Y );

if ( status ) {
  y_A[0] = 0.0;
  y_B[0] = 0.0;
  y_C[0] = 0.0;
  y_D[0] = 0.0;
  y_1[0] = 0.0;
  y_2[0] = 0.0;
  y_3[0] = 0.0;
  y_4[0] = 0.0;
}
else  {
  y_A[0] = Y[0];
  y_B[0] = Y[1];
  y_C[0] = Y[2];
  y_D[0] = Y[3];
  y_1[0] = Y[4];
  y_2[0] = Y[5];
  y_3[0] = Y[6];
  y_4[0] = Y[7];
}
#endif
/* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
}
