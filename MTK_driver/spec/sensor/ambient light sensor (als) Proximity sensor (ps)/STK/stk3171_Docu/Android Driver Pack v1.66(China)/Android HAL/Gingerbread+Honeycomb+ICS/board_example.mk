PRODUCT_PACKAGES := \
	ApiDemos      \
        SoundRecorder \
        GeneralRunner \
        EvilSkull \
        Vase \
	SenseTekSensorDisplay\
	lights.$(TARGET_PRODUCT) \
	alsa.$(TARGET_PRODUCT) \
	sensors.$(TARGET_PRODUCT)


$(call inherit-product, build/target/product/generic.mk)

PRODUCT_COPY_FILES += \
    device/SenseTek/board_example/SenseTekSensorDisplay.apk:system/app/SenseTekSensorDisplay.apk

# Overrides
PRODUCT_BRAND := SenseTek
PRODUCT_NAME := board_example
PRODUCT_DEVICE := board_example
PRODUCT_PACKAGE_OVERLAYS := device/SenseTek/board_example/overlay
