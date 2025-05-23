/*
 * Copyright (c) 2021 Vestas Wind Systems A/S
 * Copyright (c) 2018 Karsten Koenig
 * Copyright (c) 2018 Alexander Wachter
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Controller Area Network (CAN) driver API.
 */

#ifndef CAN_H_
#define CAN_H_

#include <errno.h>

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CAN Interface
 * @defgroup can_interface CAN Interface
 * @since 1.12
 * @version 1.1.0
 * @ingroup io_interfaces
 * @{
 */

/**
 * @name CAN frame definitions
 * @{
 */

/**
 * @brief Bit mask for a standard (11-bit) CAN identifier.
 */
#define CAN_STD_ID_MASK 0x7FFU
/**
 * @brief Maximum value for a standard (11-bit) CAN identifier.
 *
 * @deprecated Use ``CAN_STD_ID_MASK`` instead.
 */
#define CAN_MAX_STD_ID  CAN_STD_ID_MASK __DEPRECATED_MACRO
/**
 * @brief Bit mask for an extended (29-bit) CAN identifier.
 */
#define CAN_EXT_ID_MASK 0x1FFFFFFFU
/**
 * @brief Maximum value for an extended (29-bit) CAN identifier.
 *
 * @deprecated Use ``CAN_EXT_ID_MASK`` instead.
 */
#define CAN_MAX_EXT_ID  CAN_EXT_ID_MASK __DEPRECATED_MACRO
/**
 * @brief Maximum data length code for CAN 2.0A/2.0B.
 */
#define CAN_MAX_DLC     8U
/**
 * @brief Maximum data length code for CAN FD.
 */
#define CANFD_MAX_DLC   15U

/**
 * @cond INTERNAL_HIDDEN
 * Internally calculated maximum data length
 */
#ifndef CONFIG_CAN_FD_MODE
#define CAN_MAX_DLEN    8U
#else
#define CAN_MAX_DLEN    64U
#endif /* CONFIG_CAN_FD_MODE */

/** @endcond */

/** @} */

/**
 * @name CAN controller mode flags
 * @anchor CAN_MODE_FLAGS
 *
 * @{
 */

/** Normal mode. */
#define CAN_MODE_NORMAL     0

/** Controller is in loopback mode (receives own frames). */
#define CAN_MODE_LOOPBACK        BIT(0)

/** Controller is not allowed to send dominant bits. */
#define CAN_MODE_LISTENONLY      BIT(1)

/** Controller allows transmitting/receiving CAN FD frames. */
#define CAN_MODE_FD              BIT(2)

/** Controller does not retransmit in case of lost arbitration or missing ACK */
#define CAN_MODE_ONE_SHOT        BIT(3)

/** Controller uses triple sampling mode */
#define CAN_MODE_3_SAMPLES       BIT(4)

/** Controller requires manual recovery after entering bus-off state */
#define CAN_MODE_MANUAL_RECOVERY BIT(5)

/** @} */

struct device {
    void *config;
    void *data;
    const void *api;
};

/**
 * @brief Divide and round up.
 *
 * Example:
 * @code{.c}
 * DIV_ROUND_UP(1, 2); // 1
 * DIV_ROUND_UP(3, 2); // 2
 * @endcode
 *
 * @param n Numerator.
 * @param d Denominator.
 *
 * @return The result of @p n / @p d, rounded up.
 */
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

typedef uint32_t k_ticks_t;

typedef struct {
	k_ticks_t ticks;
} k_timeout_t;

#define K_NO_WAIT ((k_timeout_t) {0})

#ifndef BIT
#if defined(_ASMLANGUAGE)
#define BIT(n)  (1 << (n))
#else
/**
 * @brief Unsigned integer with bit position @p n set (signed in
 * assembly language).
 */
#define BIT(n)  (1UL << (n))
#endif
#endif

#define ARG_UNUSED(x) (void)(x)

/**
 * @brief Provides a type to hold CAN controller configuration flags.
 *
 * The lower 24 bits are reserved for common CAN controller mode flags. The upper 8 bits are
 * reserved for CAN controller/driver specific flags.
 *
 * @see @ref CAN_MODE_FLAGS.
 */
typedef uint32_t can_mode_t;

/**
 * @brief Defines the state of the CAN controller
 */
enum can_state {
	/** Error-active state (RX/TX error count < 96). */
	CAN_STATE_ERROR_ACTIVE,
	/** Error-warning state (RX/TX error count < 128). */
	CAN_STATE_ERROR_WARNING,
	/** Error-passive state (RX/TX error count < 256). */
	CAN_STATE_ERROR_PASSIVE,
	/** Bus-off state (RX/TX error count >= 256). */
	CAN_STATE_BUS_OFF,
	/** CAN controller is stopped and does not participate in CAN communication. */
	CAN_STATE_STOPPED,
};

/**
 * @name CAN frame flags
 * @anchor CAN_FRAME_FLAGS
 *
 * @{
 */

/** Frame uses extended (29-bit) CAN ID */
#define CAN_FRAME_IDE BIT(0)

/** Frame is a Remote Transmission Request (RTR) */
#define CAN_FRAME_RTR BIT(1)

/** Frame uses CAN FD format (FDF) */
#define CAN_FRAME_FDF BIT(2)

/** Frame uses CAN FD Baud Rate Switch (BRS). Only valid in combination with ``CAN_FRAME_FDF``. */
#define CAN_FRAME_BRS BIT(3)

/** CAN FD Error State Indicator (ESI). Indicates that the transmitting node is in error-passive
 * state. Only valid in combination with ``CAN_FRAME_FDF``.
 */
#define CAN_FRAME_ESI BIT(4)

/** @} */

/**
 * @brief CAN frame structure
 */
struct can_frame {
	/** Standard (11-bit) or extended (29-bit) CAN identifier. */
	uint32_t id;
	/** Data Length Code (DLC) indicating data length in bytes. */
	uint8_t dlc;
	/** Flags. @see @ref CAN_FRAME_FLAGS. */
	uint8_t flags;
#if defined(CONFIG_CAN_RX_TIMESTAMP) || defined(__DOXYGEN__)
	/** Captured value of the free-running timer in the CAN controller when
	 * this frame was received. The timer is incremented every bit time and
	 * captured at the start of frame bit (SOF).
	 *
	 * @note @kconfig{CONFIG_CAN_RX_TIMESTAMP} must be selected for this
	 * field to be available.
	 */
	uint16_t timestamp;
#else
	/** @cond INTERNAL_HIDDEN */
	/** Padding. */
	uint16_t reserved;
	/** @endcond */
#endif
	/** The frame payload data. */
	union {
		/** Payload data accessed as unsigned 8 bit values. */
		uint8_t data[CAN_MAX_DLEN];
		/** Payload data accessed as unsigned 32 bit values. */
		uint32_t data_32[DIV_ROUND_UP(CAN_MAX_DLEN, sizeof(uint32_t))];
	};
};

/**
 * @name CAN filter flags
 * @anchor CAN_FILTER_FLAGS
 *
 * @{
 */

/** Filter matches frames with extended (29-bit) CAN IDs */
#define CAN_FILTER_IDE  BIT(0)

/** @} */

/**
 * @brief CAN filter structure
 */
struct can_filter {
	/** CAN identifier to match. */
	uint32_t id;
	/** CAN identifier matching mask. If a bit in this mask is 0, the value
	 * of the corresponding bit in the ``id`` field is ignored by the filter.
	 */
	uint32_t mask;
	/** Flags. @see @ref CAN_FILTER_FLAGS. */
	uint8_t flags;
};

/**
 * @brief CAN controller error counters
 */
struct can_bus_err_cnt {
	/** Value of the CAN controller transmit error counter. */
	uint8_t tx_err_cnt;
	/** Value of the CAN controller receive error counter. */
	uint8_t rx_err_cnt;
};

/**
 * @brief CAN bus timing structure
 *
 * This struct is used to pass bus timing values to the configuration and
 * bitrate calculation functions.
 *
 * The propagation segment represents the time of the signal propagation. Phase
 * segment 1 and phase segment 2 define the sampling point. The ``prop_seg`` and
 * ``phase_seg1`` values affect the sampling point in the same way and some
 * controllers only have a register for the sum of those two. The sync segment
 * always has a length of 1 time quantum (see below).
 *
 * @code{.text}
 *
 * +---------+----------+------------+------------+
 * |sync_seg | prop_seg | phase_seg1 | phase_seg2 |
 * +---------+----------+------------+------------+
 *                                   ^
 *                             Sampling-Point
 *
 * @endcode
 *
 * 1 time quantum (tq) has the length of 1/(core_clock / prescaler). The bitrate
 * is defined by the core clock divided by the prescaler and the sum of the
 * segments:
 *
 *   br = (core_clock / prescaler) / (1 + prop_seg + phase_seg1 + phase_seg2)
 *
 * The Synchronization Jump Width (SJW) defines the amount of time quanta the
 * sample point can be moved. The sample point is moved when resynchronization
 * is needed.
 */
struct can_timing {
	/** Synchronisation jump width. */
	uint16_t sjw;
	/** Propagation segment. */
	uint16_t prop_seg;
	/** Phase segment 1. */
	uint16_t phase_seg1;
	/** Phase segment 2. */
	uint16_t phase_seg2;
	/** Prescaler value. */
	uint16_t prescaler;
};

/**
 * @brief Defines the application callback handler function signature
 *
 * @param dev       Pointer to the device structure for the driver instance.
 * @param error     Status of the performed send operation. See the list of
 *                  return values for @a can_send() for value descriptions.
 * @param user_data User data provided when the frame was sent.
 */
typedef void (*can_tx_callback_t)(const struct device *dev, int error, void *user_data);

/**
 * @brief Defines the application callback handler function signature for receiving.
 *
 * @param dev       Pointer to the device structure for the driver instance.
 * @param frame     Received frame.
 * @param user_data User data provided when the filter was added.
 */
typedef void (*can_rx_callback_t)(const struct device *dev, struct can_frame *frame,
				  void *user_data);

/**
 * @brief Defines the state change callback handler function signature
 *
 * @param dev       Pointer to the device structure for the driver instance.
 * @param state     State of the CAN controller.
 * @param err_cnt   CAN controller error counter values.
 * @param user_data User data provided the callback was set.
 */
typedef void (*can_state_change_callback_t)(const struct device *dev,
					    enum can_state state,
					    struct can_bus_err_cnt err_cnt,
					    void *user_data);

/**
 * @cond INTERNAL_HIDDEN
 *
 * For internal driver use only, skip these in public documentation.
 */

/**
 * @brief Calculate Transmitter Delay Compensation Offset from data phase timing parameters.
 *
 * Calculates the TDC Offset in minimum time quanta (mtq) using the sample point and CAN core clock
 * prescaler specified by a set of data phase timing parameters.
 *
 * The result is clamped to the minimum/maximum supported TDC Offset values provided.
 *
 * @param _timing_data Pointer to data phase timing parameters.
 * @param _tdco_min    Minimum supported TDC Offset value in mtq.
 * @param _tdco_max    Maximum supported TDC Offset value in mtq.
 * @return             Calculated TDC Offset value in mtq.
 */
#define CAN_CALC_TDCO(_timing_data, _tdco_min, _tdco_max)                                          \
	CLAMP((1U + _timing_data->prop_seg + _timing_data->phase_seg1) * _timing_data->prescaler,  \
	      _tdco_min, _tdco_max)

/**
 * @brief Common CAN controller driver configuration.
 *
 * This structure is common to all CAN controller drivers and is expected to be the first element in
 * the object pointed to by the config field in the device structure.
 */
struct can_driver_config {
	/** Pointer to the device structure for the associated CAN transceiver device or NULL. */
	const struct device *phy;
	/** The minimum bitrate supported by the CAN controller/transceiver combination. */
	uint32_t min_bitrate;
	/** The maximum bitrate supported by the CAN controller/transceiver combination. */
	uint32_t max_bitrate;
	/** Initial CAN classic/CAN FD arbitration phase bitrate. */
	uint32_t bitrate;
	/** Initial CAN classic/CAN FD arbitration phase sample point in permille. */
	uint16_t sample_point;
#ifdef CONFIG_CAN_FD_MODE
	/** Initial CAN FD data phase sample point in permille. */
	uint16_t sample_point_data;
	/** Initial CAN FD data phase bitrate. */
	uint32_t bitrate_data;
#endif /* CONFIG_CAN_FD_MODE */
};

/**
 * @brief Static initializer for @p can_driver_config struct
 *
 * @param node_id Devicetree node identifier
 * @param _min_bitrate minimum bitrate supported by the CAN controller
 * @param _max_bitrate maximum bitrate supported by the CAN controller
 */
#define CAN_DT_DRIVER_CONFIG_GET(node_id, _min_bitrate, _max_bitrate)				\
	{											\
		.phy = DEVICE_DT_GET_OR_NULL(DT_PHANDLE(node_id, phys)),			\
		.min_bitrate = DT_CAN_TRANSCEIVER_MIN_BITRATE(node_id, _min_bitrate),		\
		.max_bitrate = DT_CAN_TRANSCEIVER_MAX_BITRATE(node_id, _max_bitrate),		\
		.bitrate = DT_PROP_OR(node_id, bitrate,						\
			DT_PROP_OR(node_id, bus_speed, CONFIG_CAN_DEFAULT_BITRATE)),            \
		.sample_point = DT_PROP_OR(node_id, sample_point, 0),				\
		IF_ENABLED(CONFIG_CAN_FD_MODE,							\
			(.bitrate_data = DT_PROP_OR(node_id, bitrate_data,                      \
			 DT_PROP_OR(node_id, bus_speed_data, CONFIG_CAN_DEFAULT_BITRATE_DATA)), \
			 .sample_point_data = DT_PROP_OR(node_id, sample_point_data, 0),))	\
	}

/**
 * @brief Static initializer for @p can_driver_config struct from DT_DRV_COMPAT instance
 *
 * @param inst DT_DRV_COMPAT instance number
 * @param _min_bitrate minimum bitrate supported by the CAN controller
 * @param _max_bitrate maximum bitrate supported by the CAN controller
 * @see CAN_DT_DRIVER_CONFIG_GET()
 */
#define CAN_DT_DRIVER_CONFIG_INST_GET(inst, _min_bitrate, _max_bitrate)				\
	CAN_DT_DRIVER_CONFIG_GET(DT_DRV_INST(inst), _min_bitrate, _max_bitrate)

/**
 * @brief Common CAN controller driver data.
 *
 * This structure is common to all CAN controller drivers and is expected to be the first element in
 * the driver's struct driver_data declaration.
 */
struct can_driver_data {
	/** Current CAN controller mode. */
	can_mode_t mode;
	/** True if the CAN controller is started, false otherwise. */
	bool started;
	/** State change callback function pointer or NULL. */
	can_state_change_callback_t state_change_cb;
	/** State change callback user data pointer or NULL. */
	void *state_change_cb_user_data;
};

/**
 * @brief Callback API upon setting CAN bus timing
 * See @a can_set_timing() for argument description
 */
typedef int (*can_set_timing_t)(const struct device *dev,
				const struct can_timing *timing);

/**
 * @brief Optional callback API upon setting CAN FD bus timing for the data phase.
 * See @a can_set_timing_data() for argument description
 */
typedef int (*can_set_timing_data_t)(const struct device *dev,
				     const struct can_timing *timing_data);

/**
 * @brief Callback API upon getting CAN controller capabilities
 * See @a can_get_capabilities() for argument description
 */
typedef int (*can_get_capabilities_t)(const struct device *dev, can_mode_t *cap);

/**
 * @brief Callback API upon starting CAN controller
 * See @a can_start() for argument description
 */
typedef int (*can_start_t)(const struct device *dev);

/**
 * @brief Callback API upon stopping CAN controller
 * See @a can_stop() for argument description
 */
typedef int (*can_stop_t)(const struct device *dev);

/**
 * @brief Callback API upon setting CAN controller mode
 * See @a can_set_mode() for argument description
 */
typedef int (*can_set_mode_t)(const struct device *dev, can_mode_t mode);

/**
 * @brief Callback API upon sending a CAN frame
 * See @a can_send() for argument description
 *
 * @note From a driver perspective `callback` will never be `NULL` as a default callback will be
 * provided if none is provided by the caller. This allows for simplifying the driver handling.
 */
typedef int (*can_send_t)(const struct device *dev,
			  const struct can_frame *frame,
			  k_timeout_t timeout, can_tx_callback_t callback,
			  void *user_data);

/**
 * @brief Callback API upon adding an RX filter
 * See @a can_add_rx_callback() for argument description
 */
typedef int (*can_add_rx_filter_t)(const struct device *dev,
				   can_rx_callback_t callback,
				   void *user_data,
				   const struct can_filter *filter);

/**
 * @brief Callback API upon removing an RX filter
 * See @a can_remove_rx_filter() for argument description
 */
typedef void (*can_remove_rx_filter_t)(const struct device *dev, int filter_id);

/**
 * @brief Optional callback API upon manually recovering the CAN controller from bus-off state
 * See @a can_recover() for argument description
 */
typedef int (*can_recover_t)(const struct device *dev, k_timeout_t timeout);

/**
 * @brief Callback API upon getting the CAN controller state
 * See @a can_get_state() for argument description
 */
typedef int (*can_get_state_t)(const struct device *dev, enum can_state *state,
			       struct can_bus_err_cnt *err_cnt);

/**
 * @brief Callback API upon setting a state change callback
 * See @a can_set_state_change_callback() for argument description
 */
typedef void(*can_set_state_change_callback_t)(const struct device *dev,
					       can_state_change_callback_t callback,
					       void *user_data);

/**
 * @brief Callback API upon getting the CAN core clock rate
 * See @a can_get_core_clock() for argument description
 */
typedef int (*can_get_core_clock_t)(const struct device *dev, uint32_t *rate);

/**
 * @brief Optional callback API upon getting the maximum number of concurrent CAN RX filters
 * See @a can_get_max_filters() for argument description
 */
typedef int (*can_get_max_filters_t)(const struct device *dev, bool ide);

struct can_driver_api {
	can_get_capabilities_t get_capabilities;
	can_start_t start;
	can_stop_t stop;
	can_set_mode_t set_mode;
	can_set_timing_t set_timing;
	can_send_t send;
	can_add_rx_filter_t add_rx_filter;
	can_remove_rx_filter_t remove_rx_filter;
#if defined(CONFIG_CAN_MANUAL_RECOVERY_MODE) || defined(__DOXYGEN__)
	can_recover_t recover;
#endif /* CONFIG_CAN_MANUAL_RECOVERY_MODE */
	can_get_state_t get_state;
	can_set_state_change_callback_t set_state_change_callback;
	can_get_core_clock_t get_core_clock;
	can_get_max_filters_t get_max_filters;
	/* Min values for the timing registers */
	struct can_timing timing_min;
	/* Max values for the timing registers */
	struct can_timing timing_max;
#if defined(CONFIG_CAN_FD_MODE) || defined(__DOXYGEN__)
	can_set_timing_data_t set_timing_data;
	/* Min values for the timing registers during the data phase */
	struct can_timing timing_data_min;
	/* Max values for the timing registers during the data phase */
	struct can_timing timing_data_max;
#endif /* CONFIG_CAN_FD_MODE */
};

/** @endcond */

/**
 * @name CAN controller configuration
 *
 * @{
 */

/**
 * @brief Get the CAN core clock rate
 *
 * Returns the CAN core clock rate. One minimum time quantum (mtq) is 1/(core clock rate). The CAN
 * core clock can be further divided by the CAN clock prescaler (see the @a can_timing struct),
 * providing the time quantum (tq).
 *
 * @param dev  Pointer to the device structure for the driver instance.
 * @param[out] rate CAN core clock rate in Hz.
 *
 * @return 0 on success, or a negative error code on error
 */
static inline int can_get_core_clock(const struct device *dev, uint32_t *rate)
{
	const struct can_driver_api *api = (const struct can_driver_api *)dev->api;

	return api->get_core_clock(dev, rate);
}

/**
 * @brief Get minimum supported bitrate
 *
 * Get the minimum supported bitrate for the CAN controller/transceiver combination.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @return Minimum supported bitrate in bits/s
 */
static inline uint32_t can_get_bitrate_min(const struct device *dev)
{
	const struct can_driver_config *common = (const struct can_driver_config *)dev->config;

	return common->min_bitrate;
}

/**
 * @brief Get minimum supported bitrate
 *
 * Get the minimum supported bitrate for the CAN controller/transceiver combination.
 *
 * @deprecated Use @a can_get_bitrate_min() instead.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @param[out] min_bitrate Minimum supported bitrate in bits/s
 *
 * @retval -EIO General input/output error.
 * @retval -ENOSYS If this function is not implemented by the driver.
 */
static inline int can_get_min_bitrate(const struct device *dev, uint32_t *min_bitrate)
{
	*min_bitrate = can_get_bitrate_min(dev);

	return 0;
}

/**
 * @brief Get maximum supported bitrate
 *
 * Get the maximum supported bitrate for the CAN controller/transceiver combination.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @return Maximum supported bitrate in bits/s
 */
static inline uint32_t can_get_bitrate_max(const struct device *dev)
{
	const struct can_driver_config *common = (const struct can_driver_config *)dev->config;

	return common->max_bitrate;
}

/**
 * @brief Get maximum supported bitrate
 *
 * Get the maximum supported bitrate for the CAN controller/transceiver combination.
 *
 * @deprecated Use @a can_get_bitrate_max() instead.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @param[out] max_bitrate Maximum supported bitrate in bits/s
 *
 * @retval 0 If successful.
 * @retval -EIO General input/output error.
 * @retval -ENOSYS If this function is not implemented by the driver.
 */
static inline int can_get_max_bitrate(const struct device *dev, uint32_t *max_bitrate)
{
	*max_bitrate = can_get_bitrate_max(dev);

	return 0;
}

/**
 * @brief Get the minimum supported timing parameter values.
 *
 * @param dev Pointer to the device structure for the driver instance.
 *
 * @return Pointer to the minimum supported timing parameter values.
 */
static inline const struct can_timing *can_get_timing_min(const struct device *dev)
{
	const struct can_driver_api *api = (const struct can_driver_api *)dev->api;

	return &api->timing_min;
}

/**
 * @brief Get the maximum supported timing parameter values.
 *
 * @param dev Pointer to the device structure for the driver instance.
 *
 * @return Pointer to the maximum supported timing parameter values.
 */
static inline const struct can_timing *can_get_timing_max(const struct device *dev)
{
	const struct can_driver_api *api = (const struct can_driver_api *)dev->api;

	return &api->timing_max;
}

/**
 * @brief Calculate timing parameters from bitrate and sample point
 *
 * Calculate the timing parameters from a given bitrate in bits/s and the
 * sampling point in permill (1/1000) of the entire bit time. The bitrate must
 * always match perfectly. If no result can be reached for the given parameters,
 * -EINVAL is returned.
 *
 * If the sample point is set to 0, this function defaults to a sample point of 75.0%
 * for bitrates over 800 kbit/s, 80.0% for bitrates over 500 kbit/s, and 87.5% for
 * all other bitrates.
 *
 * @note The requested ``sample_pnt`` will not always be matched perfectly. The
 * algorithm calculates the best possible match.
 *
 * @param dev        Pointer to the device structure for the driver instance.
 * @param[out] res   Result is written into the @a can_timing struct provided.
 * @param bitrate    Target bitrate in bits/s.
 * @param sample_pnt Sample point in permille of the entire bit time or 0 for
 *                   automatic sample point location.
 *
 * @retval 0 or positive sample point error on success.
 * @retval -EINVAL if the requested bitrate or sample point is out of range.
 * @retval -ENOTSUP if the requested bitrate is not supported.
 * @retval -EIO if @a can_get_core_clock() is not available.
 */
 int can_calc_timing(const struct device *dev, struct can_timing *res,
			      uint32_t bitrate, uint16_t sample_pnt);

/**
 * @brief Get the minimum supported timing parameter values for the data phase.
 *
 * Same as @a can_get_timing_min() but for the minimum values for the data phase.
 *
 * @note @kconfig{CONFIG_CAN_FD_MODE} must be selected for this function to be
 * available.
 *
 * @param dev Pointer to the device structure for the driver instance.
 *
 * @return Pointer to the minimum supported timing parameter values, or NULL if
 *         CAN FD support is not implemented by the driver.
 */
 const struct can_timing *can_get_timing_data_min(const struct device *dev);

/**
 * @brief Get the maximum supported timing parameter values for the data phase.
 *
 * Same as @a can_get_timing_max() but for the maximum values for the data phase.
 *
 * @note @kconfig{CONFIG_CAN_FD_MODE} must be selected for this function to be
 * available.
 *
 * @param dev Pointer to the device structure for the driver instance.
 *
 * @return Pointer to the maximum supported timing parameter values, or NULL if
 *         CAN FD support is not implemented by the driver.
 */
 const struct can_timing *can_get_timing_data_max(const struct device *dev);

/**
 * @brief Calculate timing parameters for the data phase
 *
 * Same as @a can_calc_timing() but with the maximum and minimum values from the
 * data phase.
 *
 * @note @kconfig{CONFIG_CAN_FD_MODE} must be selected for this function to be
 * available.
 *
 * @param dev        Pointer to the device structure for the driver instance.
 * @param[out] res   Result is written into the @a can_timing struct provided.
 * @param bitrate    Target bitrate for the data phase in bits/s
 * @param sample_pnt Sample point for the data phase in permille of the entire bit
 *                   time or 0 for automatic sample point location.
 *
 * @retval 0 or positive sample point error on success.
 * @retval -EINVAL if the requested bitrate or sample point is out of range.
 * @retval -ENOTSUP if the requested bitrate is not supported.
 * @retval -EIO if @a can_get_core_clock() is not available.
 */
 int can_calc_timing_data(const struct device *dev, struct can_timing *res,
				   uint32_t bitrate, uint16_t sample_pnt);

/**
 * @brief Configure the bus timing for the data phase of a CAN FD controller.
 *
 * @note @kconfig{CONFIG_CAN_FD_MODE} must be selected for this function to be
 * available.
 *
 * @see can_set_timing()
 *
 * @param dev         Pointer to the device structure for the driver instance.
 * @param timing_data Bus timings for data phase
 *
 * @retval 0 If successful.
 * @retval -EBUSY if the CAN controller is not in stopped state.
 * @retval -EIO General input/output error, failed to configure device.
 * @retval -ENOTSUP if the timing parameters are not supported by the driver.
 * @retval -ENOSYS if CAN FD support is not implemented by the driver.
 */
 int can_set_timing_data(const struct device *dev,
				  const struct can_timing *timing_data);

/**
 * @brief Set the bitrate for the data phase of the CAN FD controller
 *
 * CAN in Automation (CiA) 301 v4.2.0 recommends a sample point location of
 * 87.5% percent for all bitrates. However, some CAN controllers have
 * difficulties meeting this for higher bitrates.
 *
 * This function defaults to using a sample point of 75.0% for bitrates over 800
 * kbit/s, 80.0% for bitrates over 500 kbit/s, and 87.5% for all other
 * bitrates. This is in line with the sample point locations used by the Linux
 * kernel.
 *
 * @note @kconfig{CONFIG_CAN_FD_MODE} must be selected for this function to be
 * available.
 *
 * @see can_set_bitrate()

 * @param dev          Pointer to the device structure for the driver instance.
 * @param bitrate_data Desired data phase bitrate.
 *
 * @retval 0 If successful.
 * @retval -EBUSY if the CAN controller is not in stopped state.
 * @retval -EINVAL if the requested bitrate is out of range.
 * @retval -ENOTSUP if the requested bitrate not supported by the CAN controller/transceiver
 *                  combination.
 * @retval -ERANGE if the resulting sample point is off by more than +/- 5%.
 * @retval -EIO General input/output error, failed to set bitrate.
 */
 int can_set_bitrate_data(const struct device *dev, uint32_t bitrate_data);

/**
 * @brief Fill in the prescaler value for a given bitrate and timing
 *
 * Fill the prescaler value in the timing struct. The sjw, prop_seg, phase_seg1
 * and phase_seg2 must be given.
 *
 * The returned bitrate error is remainder of the division of the clock rate by
 * the bitrate times the timing segments.
 *
 *
 * @param dev     Pointer to the device structure for the driver instance.
 * @param timing  Result is written into the can_timing struct provided.
 * @param bitrate Target bitrate.
 *
 * @retval 0 or positive bitrate error.
 * @retval Negative error code on error.
 */
int can_calc_prescaler(const struct device *dev, struct can_timing *timing,
				    uint32_t bitrate);

/**
 * @brief Configure the bus timing of a CAN controller.
 *
 * @see can_set_timing_data()
 *
 * @param dev         Pointer to the device structure for the driver instance.
 * @param timing      Bus timings.
 *
 * @retval 0 If successful.
 * @retval -EBUSY if the CAN controller is not in stopped state.
 * @retval -ENOTSUP if the timing parameters are not supported by the driver.
 * @retval -EIO General input/output error, failed to configure device.
 */
 int can_set_timing(const struct device *dev,
			     const struct can_timing *timing);

/**
 * @brief Get the supported modes of the CAN controller
 *
 * The returned capabilities may not necessarily be supported at the same time (e.g. some CAN
 * controllers support both ``CAN_MODE_LOOPBACK`` and ``CAN_MODE_LISTENONLY``, but not at the same
 * time).
 *
 * @param dev      Pointer to the device structure for the driver instance.
 * @param[out] cap Supported capabilities.
 *
 * @retval 0 If successful.
 * @retval -EIO General input/output error, failed to get capabilities.
 */
static inline int can_get_capabilities(const struct device *dev, can_mode_t *cap)
{
	const struct can_driver_api *api = (const struct can_driver_api *)dev->api;

	return api->get_capabilities(dev, cap);
}

/**
 * @brief Start the CAN controller
 *
 * Bring the CAN controller out of `CAN_STATE_STOPPED`. This will reset the RX/TX error counters,
 * enable the CAN controller to participate in CAN communication, and enable the CAN transceiver, if
 * supported.
 *
 * Starting the CAN controller resets all the CAN controller statistics.
 *
 * @see can_stop()
 * @see can_transceiver_enable()
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @retval 0 if successful.
 * @retval -EALREADY if the device is already started.
 * @retval -EIO General input/output error, failed to start device.
 */
static inline int can_start(const struct device *dev)
{
	const struct can_driver_api *api = (const struct can_driver_api *)dev->api;

	return api->start(dev);
}

/**
 * @brief Stop the CAN controller
 *
 * Bring the CAN controller into `CAN_STATE_STOPPED`. This will disallow the CAN controller from
 * participating in CAN communication, abort any pending CAN frame transmissions, and disable the
 * CAN transceiver, if supported.
 *
 * @see can_start()
 * @see can_transceiver_disable()
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @retval 0 if successful.
 * @retval -EALREADY if the device is already stopped.
 * @retval -EIO General input/output error, failed to stop device.
 */
static inline int can_stop(const struct device *dev)
{
	const struct can_driver_api *api = (const struct can_driver_api *)dev->api;

	return api->stop(dev);
}

/**
 * @brief Set the CAN controller to the given operation mode
 *
 * @param dev  Pointer to the device structure for the driver instance.
 * @param mode Operation mode.
 *
 * @retval 0 If successful.
 * @retval -EBUSY if the CAN controller is not in stopped state.
 * @retval -EIO General input/output error, failed to configure device.
 */
static inline int can_set_mode(const struct device *dev, can_mode_t mode)
{
	const struct can_driver_api *api = (const struct can_driver_api *)dev->api;

	return api->set_mode(dev, mode);
}

/**
 * @brief Get the operation mode of the CAN controller
 *
 * @param dev Pointer to the device structure for the driver instance.
 *
 * @return Current operation mode.
 */
static inline can_mode_t can_get_mode(const struct device *dev)
{
	const struct can_driver_data *common = (const struct can_driver_data *)dev->data;

	return common->mode;
}

/**
 * @brief Set the bitrate of the CAN controller
 *
 * CAN in Automation (CiA) 301 v4.2.0 recommends a sample point location of
 * 87.5% percent for all bitrates. However, some CAN controllers have
 * difficulties meeting this for higher bitrates.
 *
 * This function defaults to using a sample point of 75.0% for bitrates over 800
 * kbit/s, 80.0% for bitrates over 500 kbit/s, and 87.5% for all other
 * bitrates. This is in line with the sample point locations used by the Linux
 * kernel.
 *
 * @see can_set_bitrate_data()
 *
 * @param dev          Pointer to the device structure for the driver instance.
 * @param bitrate      Desired arbitration phase bitrate.
 *
 * @retval 0 If successful.
 * @retval -EBUSY if the CAN controller is not in stopped state.
 * @retval -EINVAL if the requested bitrate is out of range.
 * @retval -ENOTSUP if the requested bitrate not supported by the CAN controller/transceiver
 *                  combination.
 * @retval -ERANGE if the resulting sample point is off by more than +/- 5%.
 * @retval -EIO General input/output error, failed to set bitrate.
 */
 int can_set_bitrate(const struct device *dev, uint32_t bitrate);

/** @} */

/**
 * @name Transmitting CAN frames
 *
 * @{
 */

/**
 * @brief Queue a CAN frame for transmission on the CAN bus
 *
 * Queue a CAN frame for transmission on the CAN bus with optional timeout and
 * completion callback function.
 *
 * Queued CAN frames are transmitted in order according to the their priority:
 * - The lower the CAN-ID, the higher the priority.
 * - Data frames have higher priority than Remote Transmission Request (RTR)
 *   frames with identical CAN-IDs.
 * - Frames with standard (11-bit) identifiers have higher priority than frames
 *   with extended (29-bit) identifiers with identical base IDs (the higher 11
 *   bits of the extended identifier).
 * - Transmission order for queued frames with the same priority is hardware
 *   dependent.
 *
 * @note If transmitting segmented messages spanning multiple CAN frames with
 * identical CAN-IDs, the sender must ensure to only queue one frame at a time
 * if FIFO order is required.
 *
 * By default, the CAN controller will automatically retry transmission in case
 * of lost bus arbitration or missing acknowledge. Some CAN controllers support
 * disabling automatic retransmissions via ``CAN_MODE_ONE_SHOT``.
 *
 * @param dev       Pointer to the device structure for the driver instance.
 * @param frame     CAN frame to transmit.
 * @param timeout   Timeout waiting for a empty TX mailbox or ``K_FOREVER``.
 * @param callback  Optional callback for when the frame was sent or a
 *                  transmission error occurred. If ``NULL``, this function is
 *                  blocking until frame is sent. The callback must be ``NULL``
 *                  if called from user mode.
 * @param user_data User data to pass to callback function.
 *
 * @retval 0 if successful.
 * @retval -EINVAL if an invalid parameter was passed to the function.
 * @retval -ENOTSUP if an unsupported parameter was passed to the function.
 * @retval -ENETDOWN if the CAN controller is in stopped state.
 * @retval -ENETUNREACH if the CAN controller is in bus-off state.
 * @retval -EBUSY if CAN bus arbitration was lost (only applicable if automatic
 *                retransmissions are disabled).
 * @retval -EIO if a general transmit error occurred (e.g. missing ACK if
 *              automatic retransmissions are disabled).
 * @retval -EAGAIN on timeout.
 */
 int can_send(const struct device *dev, const struct can_frame *frame,
		       k_timeout_t timeout, can_tx_callback_t callback,
		       void *user_data);

/** @} */

/**
 * @name Receiving CAN frames
 *
 * @{
 */

/**
 * @brief Add a callback function for a given CAN filter
 *
 * Add a callback to CAN identifiers specified by a filter. When a received CAN
 * frame matching the filter is received by the CAN controller, the callback
 * function is called in interrupt context.
 *
 * If a received frame matches more than one filter (i.e., the filter IDs/masks or
 * flags overlap), the priority of the match is hardware dependent.
 *
 * The same callback function can be used for multiple filters.
 *
 * @param dev       Pointer to the device structure for the driver instance.
 * @param callback  This function is called by the CAN controller driver whenever
 *                  a frame matching the filter is received.
 * @param user_data User data to pass to callback function.
 * @param filter    Pointer to a @a can_filter structure defining the filter.
 *
 * @retval filter_id on success.
 * @retval -ENOSPC if there are no free filters.
 * @retval -EINVAL if the requested filter type is invalid.
 * @retval -ENOTSUP if the requested filter type is not supported.
 */
int can_add_rx_filter(const struct device *dev, can_rx_callback_t callback,
		      void *user_data, const struct can_filter *filter);

/**
 * @brief Remove a CAN RX filter
 *
 * This routine removes a CAN RX filter based on the filter ID returned by @a
 * can_add_rx_filter() or @a can_add_rx_filter_msgq().
 *
 * @param dev       Pointer to the device structure for the driver instance.
 * @param filter_id Filter ID
 */
static inline void can_remove_rx_filter(const struct device *dev, int filter_id)
{
	const struct can_driver_api *api = (const struct can_driver_api *)dev->api;

	api->remove_rx_filter(dev, filter_id);
}

/**
 * @brief Get maximum number of RX filters
 *
 * Get the maximum number of concurrent RX filters for the CAN controller.
 *
 * @param dev Pointer to the device structure for the driver instance.
 * @param ide Get the maximum standard (11-bit) CAN ID filters if false, or extended (29-bit) CAN ID
 *            filters if true.
 *
 * @retval Positive number of maximum concurrent filters.
 * @retval -EIO General input/output error.
 * @retval -1 If this function is not implemented by the driver.
 */
static inline int can_get_max_filters(const struct device *dev, bool ide)
{
	const struct can_driver_api *api = (const struct can_driver_api *)dev->api;

	if (api->get_max_filters == NULL) {
		return -1;
	}

	return api->get_max_filters(dev, ide);
}

/** @} */

/**
 * @name CAN bus error reporting and handling
 *
 * @{
 */

/**
 * @brief Get current CAN controller state
 *
 * Returns the current state and optionally the error counter values of the CAN
 * controller.
 *
 * @param dev          Pointer to the device structure for the driver instance.
 * @param[out] state   Pointer to the state destination enum or NULL.
 * @param[out] err_cnt Pointer to the err_cnt destination structure or NULL.
 *
 * @retval 0 If successful.
 * @retval -EIO General input/output error, failed to get state.
 */
static inline int can_get_state(const struct device *dev, enum can_state *state,
				       struct can_bus_err_cnt *err_cnt)
{
	const struct can_driver_api *api = (const struct can_driver_api *)dev->api;

	return api->get_state(dev, state, err_cnt);
}

/**
 * @brief Set a callback for CAN controller state change events
 *
 * Set the callback for CAN controller state change events. The callback
 * function will be called in interrupt context.
 *
 * Only one callback can be registered per controller. Calling this function
 * again overrides any previously registered callback.
 *
 * @param dev       Pointer to the device structure for the driver instance.
 * @param callback  Callback function.
 * @param user_data User data to pass to callback function.
 */
static inline void can_set_state_change_callback(const struct device *dev,
						 can_state_change_callback_t callback,
						 void *user_data)
{
	const struct can_driver_api *api = (const struct can_driver_api *)dev->api;

	api->set_state_change_callback(dev, callback, user_data);
}

/**
 * @brief Convert from Data Length Code (DLC) to the number of data bytes
 *
 * @param dlc Data Length Code (DLC).
 *
 * @retval Number of bytes.
 */
static inline uint8_t can_dlc_to_bytes(uint8_t dlc)
{
	static const uint8_t dlc_table[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12,
					    16, 20, 24, 32, 48, 64};

	return dlc_table[MIN(dlc, ARRAY_SIZE(dlc_table) - 1)];
}

/**
 * @brief Convert from number of bytes to Data Length Code (DLC)
 *
 * @param num_bytes Number of bytes.
 *
 * @retval Data Length Code (DLC).
 */
static inline uint8_t can_bytes_to_dlc(uint8_t num_bytes)
{
	return num_bytes <= 8  ? num_bytes :
	       num_bytes <= 12 ? 9 :
	       num_bytes <= 16 ? 10 :
	       num_bytes <= 20 ? 11 :
	       num_bytes <= 24 ? 12 :
	       num_bytes <= 32 ? 13 :
	       num_bytes <= 48 ? 14 :
	       15;
}

/**
 * @brief Check if a CAN frame matches a CAN filter
 *
 * @param frame CAN frame.
 * @param filter CAN filter.
 * @return true if the CAN frame matches the CAN filter, false otherwise
 */
static inline bool can_frame_matches_filter(const struct can_frame *frame,
					    const struct can_filter *filter)
{
	if ((frame->flags & CAN_FRAME_IDE) != 0 && (filter->flags & CAN_FILTER_IDE) == 0) {
		/* Extended (29-bit) ID frame, standard (11-bit) filter */
		return false;
	}

	if ((frame->flags & CAN_FRAME_IDE) == 0 && (filter->flags & CAN_FILTER_IDE) != 0) {
		/* Standard (11-bit) ID frame, extended (29-bit) filter */
		return false;
	}

	if ((frame->id ^ filter->id) & filter->mask) {
		/* Masked ID mismatch */
		return false;
	}

	return true;
}

/** @} */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* CAN_H_ */
