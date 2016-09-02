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
  *   Created: Fri Nov 28 09:55:45 2014
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
#include "ev314.h"
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

#define EV314_NB_EV314_DEVICES				4						// Maximum number of attached EV314 devices

#define EV314_USB_TIMEOUT             1000   			// Milliseconds
#define EV314_INTERFACE_NUMBER        0
#define EV314_CONFIGURATION_NB        1
#define EV314_EP_OUT                  0x01
#define EV314_EP_IN                   0x81
#define EV314_PACKET_SIZE							0x400
#define EV314_RESET_RESPONSE_SIZE     5
#define EV314_POWER_RESPONSE_SIZE     5
#define EV314_VENDOR_LEGO             0x0694
#define EV314_PRODUCT_EV3             0x0005

/* Error codes */
#define EV314_OK                      0
#define EV314_USB_ERROR								1
#define EV314_NOT_PRESENT             2
#define EV314_CONFIGURATION_ERROR     3
#define EV314_IN_USE                  4
#define EV314_USB_WRITE_ERROR         5
#define EV314_USB_READ_ERROR          6
#define EV314_USB_PARTIAL_TRANS				7
#define EV314_USB_OVERFLOW						9
#define EV314_BYTECODE_ERROR          10
#define EV314_INTERNAL_ERROR          11

typedef int                         	EV314_error_t;

struct timespec												profiling_start;
unsigned long long										EV314_serial_list[EV314_NB_EV314_DEVICES];
unsigned char													EV314_nb_devices = 0;
struct libusb_device_handle						*EV314_hdl[EV314_NB_EV314_DEVICES];
unsigned char                         EV314_initialized = 0;

/*
 * ev314_profiling_start: start timer
 * 
 */
void ev314_profiling_start( void )	{
	clock_gettime( CLOCK_MONOTONIC, &profiling_start );
}

/*
 * ev314_profiling_stop: stop timer and print time
 * 
 */
void ev314_profiling_stop( void )	{
	struct timespec						profiling_stop;
	
	clock_gettime( CLOCK_MONOTONIC, &profiling_stop );
	
	printf( 	"** Profiling duration: %d us.\n",
						(int)( ( profiling_stop.tv_sec - profiling_start.tv_sec ) * 1000000
								 + ( profiling_stop.tv_nsec - profiling_start.tv_nsec ) / 1000 ) );
}

/*
 * USB API.
 * 
 */
EV314_error_t EV314_init( void )  {
	libusb_device 									*dev, **devs;
	struct libusb_device_descriptor	desc;
	struct libusb_device_handle			*EV314_current_hdl = NULL;
	int															i, j, status;
  char														serial[EV314_LENGTH_SERIAL+1];
  char														serial_hex[EV314_LENGTH_SERIAL+3];
  unsigned long long							EV314_serial_tmp;
  
  if ( EV314_initialized )
    return EV314_OK;
	EV314_initialized = 1;
  
  status = libusb_init( NULL );
  
	if ( status ) {
    EV314_initialized = 0;
		return EV314_USB_ERROR;
  }
	
	/* Initialize usb serial number list and opened handles list */
	
	EV314_nb_devices = 0;
	for ( i = 0; i < EV314_NB_EV314_DEVICES; i++ )
		EV314_hdl[i] = NULL;

	if ( libusb_get_device_list( NULL, &devs ) < 0 )  {
    EV314_initialized = 0;
		return EV314_USB_ERROR;
  }
	
	/* 
	 * Go through device list loooking for EV3 devices.
	 * The EV3 devices will be sorted with increasing ID number.
	 */
	
	for ( i = 0; ( dev = devs[i] ) != NULL; i++ ) {

		status = libusb_get_device_descriptor( dev, &desc );

		if ( status >= 0 ) {
			if ( 	( desc.idVendor == EV314_VENDOR_LEGO ) &&
						( desc.idProduct == EV314_PRODUCT_EV3 ) )	{
				
				/* Open the device */
				status = libusb_open( dev, &EV314_current_hdl );
				if ( status < 0 )	{
					libusb_free_device_list( devs, 1 );
          EV314_initialized = 0;
					return EV314_USB_ERROR;
				}
				
				/* Check the serial number */
				
				status = libusb_get_string_descriptor_ascii( EV314_current_hdl, desc.iSerialNumber, (unsigned char*)serial, sizeof( serial ) );
				if ( status == EV314_LENGTH_SERIAL ) {
					
					/* EV3 found */
					
					snprintf( serial_hex, EV314_LENGTH_SERIAL + 3, "0x%s", serial );
					EV314_serial_list[EV314_nb_devices++] = strtoull( serial_hex, NULL, 0 );
					
					/* Sort the list */
					
					for ( j = EV314_nb_devices - 1; j > 0; j-- )	{
						
						if ( EV314_serial_list[j] < EV314_serial_list[j-1] )	{
							
							/* Swapping */
							
							EV314_serial_tmp = EV314_serial_list[j];
							EV314_serial_list[j] = EV314_serial_list[j-1];
							EV314_serial_list[j-1] = EV314_serial_tmp;
						}
					}
					
					if ( EV314_nb_devices == EV314_NB_EV314_DEVICES )	{
						
						/* Max nd of devices reached */
						
						libusb_close( EV314_current_hdl ); 
						libusb_free_device_list( devs, 1 );
						return EV314_OK;
					}
				}

				libusb_close( EV314_current_hdl ); 
        EV314_current_hdl = NULL;
      }
		}
	}
	
	libusb_free_device_list( devs, 1 );
  
  return EV314_OK;
}


int EV314_find_and_open( unsigned char serial_id )  {
	char														serial[EV314_LENGTH_SERIAL+1];
	char														expected_serial_hex[EV314_LENGTH_SERIAL+3];
	libusb_device 									*dev, **devs;
	struct libusb_device_descriptor	desc;
	struct libusb_device_handle			*EV314_current_hdl = NULL;
	int															i, status, transfered;
  unsigned char	                  tmpbuf[EV314_PACKET_SIZE];
  
  if ( !EV314_initialized )
    if ( EV314_init( ) )
      return EV314_CONFIGURATION_ERROR;
  
	if ( libusb_get_device_list( NULL, &devs ) < 0 )
		return EV314_USB_ERROR;
	
	if ( serial_id >= EV314_nb_devices )
		return EV314_INTERNAL_ERROR;
	
	/* Convert expected serial in char */
	
	snprintf( expected_serial_hex, EV314_LENGTH_SERIAL + 3, "%#014llx", EV314_serial_list[serial_id] );
	
	/* Go through device list loooking for an EV3 device */
	for ( i = 0; ( dev = devs[i] ) != NULL; i++ ) {

		status = libusb_get_device_descriptor( dev, &desc );

		if ( status >= 0 ) {
			if ( 	( desc.idVendor == EV314_VENDOR_LEGO ) &&
						( desc.idProduct == EV314_PRODUCT_EV3 ) )	{
				
				/* Open the device */
				status = libusb_open( dev, &EV314_current_hdl );
				if ( status < 0 )	{
					libusb_free_device_list( devs, 1 );
					return EV314_USB_ERROR;
				}
				
				/* Check the serial number */
				
				status = libusb_get_string_descriptor_ascii( EV314_current_hdl, desc.iSerialNumber, (unsigned char*)serial, sizeof( serial ) );
				if ( status == EV314_LENGTH_SERIAL ) {
					if ( strcmp( &expected_serial_hex[2], serial ) )	{
						libusb_close( EV314_current_hdl );
            EV314_current_hdl = NULL;
						continue;
					}
				}
				else {
					libusb_close( EV314_current_hdl ); 
          EV314_current_hdl = NULL;
					continue;
				}
				
				/* Detach possible kernel driver bound to interface */
  			libusb_detach_kernel_driver( EV314_current_hdl, EV314_INTERFACE_NUMBER );
  			
  			/* Claiming the interface */
				status = libusb_claim_interface( EV314_current_hdl, EV314_INTERFACE_NUMBER );
				if ( status )	{
					libusb_close( EV314_current_hdl );
					libusb_free_device_list( devs, 1 );
					return EV314_USB_ERROR;
				}
								
				/* Handle found and in good shape */
				EV314_hdl[serial_id] = EV314_current_hdl;
				
				/* Flush input buffer */				
				do {
          status = libusb_interrupt_transfer( EV314_hdl[serial_id], EV314_EP_IN, tmpbuf, EV314_PACKET_SIZE, &transfered, EV314_USB_TIMEOUT );
        } while ( ( status == 0 ) && ( transfered != 0 ) );
        
				libusb_free_device_list( devs, 1 );
        return EV314_OK;
      }
		}
	}
	
	libusb_free_device_list( devs, 1 );
  return EV314_USB_ERROR;
}

void EV314_close( unsigned char serial_id )  {
	int i;
  
  if ( !EV314_initialized )
    return;
  
  if ( serial_id >= EV314_nb_devices )
		return;
  
  if ( EV314_hdl[serial_id] )	{
    libusb_release_interface( EV314_hdl[serial_id], EV314_INTERFACE_NUMBER );
    libusb_close( EV314_hdl[serial_id] );
    EV314_hdl[serial_id] = NULL;
  }
  
  for ( i = 0; i < EV314_nb_devices; i++ )
    if ( EV314_hdl[i] )
      break;
  
  if ( i == EV314_nb_devices )  {
    EV314_nb_devices = 0;
    libusb_exit( NULL );
    EV314_initialized = 0;
  }
}

EV314_error_t EV314_send_buf( unsigned char serial_id, unsigned char *buf, int len )  {
	int	status, transfered = 0;
  
  if ( !EV314_initialized )
    return EV314_CONFIGURATION_ERROR;
  
  if ( serial_id >= EV314_nb_devices )
		return EV314_INTERNAL_ERROR;
  
  if ( EV314_hdl[serial_id] == NULL )
    return EV314_CONFIGURATION_ERROR;
  
  if ( len > EV314_PACKET_SIZE )
  	return EV314_USB_OVERFLOW;
  
  status = libusb_interrupt_transfer( EV314_hdl[serial_id], EV314_EP_OUT, buf, len, &transfered, EV314_USB_TIMEOUT );
  
  if ( status  )
    return EV314_USB_WRITE_ERROR;
  
  if ( transfered != len )
  	return EV314_USB_PARTIAL_TRANS;

  return EV314_OK;
}

EV314_error_t EV314_recv_buf( unsigned char serial_id, unsigned char *buf, int len )  {
  int						i, status, transfered = 0;
  unsigned char	tmpbuf[EV314_PACKET_SIZE];
  
  if ( !EV314_initialized )
    return EV314_CONFIGURATION_ERROR;
  
  if ( serial_id >= EV314_nb_devices )
		return EV314_INTERNAL_ERROR;
  
  if ( EV314_hdl[serial_id] == NULL )
    return EV314_CONFIGURATION_ERROR;
  
  if ( len > EV314_PACKET_SIZE )
  	return EV314_USB_OVERFLOW;
  
  status = libusb_interrupt_transfer( EV314_hdl[serial_id], EV314_EP_IN, tmpbuf, EV314_PACKET_SIZE, &transfered, EV314_USB_TIMEOUT );

  if ( status  )
    return EV314_USB_READ_ERROR;
  
  if ( transfered != EV314_PACKET_SIZE )
  	return EV314_USB_PARTIAL_TRANS;
  
  for ( i = 0; i < len; i++ )
  	buf[i] = tmpbuf[i];

  return EV314_OK;
}

EV314_error_t EV314_reset_encoders( unsigned char serial_id )  {
  struct ev314_control_struct		ev314_control;
  
  memset( &ev314_control, 0, sizeof( struct ev314_control_struct ) );
  
  ev314_control.magic = EV314_MAGIC;
	ev314_control.cmd = EV314_CMD_RESET_ENC;
  
  return EV314_send_buf( serial_id, (unsigned char*)&ev314_control, sizeof( ev314_control ) );
}

#endif
/* %%%-SFUNWIZ_wrapper_externs_Changes_END --- EDIT HERE TO _BEGIN */

/*
 * Output functions
 *
 */
void rpi_sfun_ev314_Outputs_wrapper(const real_T *UA,
                          const real_T *UB,
                          const real_T *UC,
                          const real_T *UD,
                          real_T *EA,
                          real_T *EB,
                          real_T *EC,
                          real_T *ED,
                          real_T *S1,
                          real_T *S2,
                          real_T *S3,
                          real_T *S4,
                          real_T *IBAT,
                          real_T *VBAT  , 
                          const real_T  *rpi_Ts, const int_T  p_width0, 
                          const uint8_T  *rpi_ID,  const int_T p_width1)
{
/* %%%-SFUNWIZ_wrapper_Outputs_Changes_BEGIN --- EDIT HERE TO _END */
  #ifdef MATLAB_MEX_FILE

  EA[0] = 0.0;
  EB[0] = 0.0;
  EC[0] = 0.0;
  ED[0] = 0.0;
  S1[0] = 0.0;
  S2[0] = 0.0;
  S3[0] = 0.0;
  S4[0] = 0.0;
  IBAT[0] = 0.0;
  VBAT[0] = 0.0;
  
  #else
  struct ev314_control_struct		ev314_control;
	struct ev314_state_struct			ev314_state;
  
  if ( ( p_width0 > 1 ) || ( p_width1 > 1 ) )	{
    fprintf( stderr, "** Multiple rates not allowed in this block **\n" );
    EA[0] = 0.0;
    EB[0] = 0.0;
    EC[0] = 0.0;
    ED[0] = 0.0;
    S1[0] = 0.0;
    S2[0] = 0.0;
    S3[0] = 0.0;
    S4[0] = 0.0;
    IBAT[0] = 0.0;
    VBAT[0] = 0.0;
    return;
  }

  if ( *rpi_ID >= EV314_NB_EV314_DEVICES )	{
    fprintf( stderr, "** Max NXT ID = %d **\n", EV314_NB_EV314_DEVICES - 1 );
    EA[0] = 0.0;
    EB[0] = 0.0;
    EC[0] = 0.0;
    ED[0] = 0.0;
    S1[0] = 0.0;
    S2[0] = 0.0;
    S3[0] = 0.0;
    S4[0] = 0.0;
    IBAT[0] = 0.0;
    VBAT[0] = 0.0;
    return;
  }

  if ( *rpi_Ts < 0.001 )	{
    fprintf( stderr, "** Max sampling rate = 1000Hz **\n" );
    EA[0] = 0.0;
    EB[0] = 0.0;
    EC[0] = 0.0;
    ED[0] = 0.0;
    S1[0] = 0.0;
    S2[0] = 0.0;
    S3[0] = 0.0;
    S4[0] = 0.0;
    IBAT[0] = 0.0;
    VBAT[0] = 0.0;
    return;
  }
  
  memset( &ev314_control, 0, sizeof( struct ev314_control_struct ) );
  
  ev314_control.magic = EV314_MAGIC;
	ev314_control.cmd = EV314_CMD_CONTROL;
	ev314_control.motor_power[0] = (int)UA[0];
	ev314_control.motor_power[1] = (int)UB[0];
	ev314_control.motor_power[2] = (int)UC[0];
	ev314_control.motor_power[3] = (int)UD[0];
  
  EV314_send_buf( *rpi_ID, (unsigned char*)&ev314_control, sizeof( ev314_control ) );
  memset( &ev314_state, 0, sizeof( struct ev314_state_struct ) );
  EV314_recv_buf( *rpi_ID, (unsigned char*)&ev314_state, sizeof( ev314_state ) );
  
  EA[0] = (real_T)ev314_state.motor_angle[0];
  EB[0] = (real_T)ev314_state.motor_angle[1];
  EC[0] = (real_T)ev314_state.motor_angle[2];
  ED[0] = (real_T)ev314_state.motor_angle[3];
  S1[0] = (real_T)ev314_state.input_ADC[0];
  S2[0] = (real_T)ev314_state.input_ADC[1];
  S3[0] = (real_T)ev314_state.input_ADC[2];
  S4[0] = (real_T)ev314_state.input_ADC[3];
  IBAT[0] = (real_T)ev314_state.battery_current;
  VBAT[0] = (real_T)ev314_state.battery_voltage;
  
  #endif
/* %%%-SFUNWIZ_wrapper_Outputs_Changes_END --- EDIT HERE TO _BEGIN */
}