#include "i2c_helper.h"
#include "esp_log.h"

static const char *TAG = "I2C_HELPER";

esp_err_t i2c_helper_init(const i2c_bus_cfg_t *cfg)
{
    if (!cfg) return ESP_ERR_INVALID_ARG;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = cfg->sda_io,
        .scl_io_num = cfg->scl_io,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = cfg->clk_speed,
    };

    ESP_ERROR_CHECK(i2c_param_config(cfg->port, &conf));

    esp_err_t ret = i2c_driver_install(cfg->port, I2C_MODE_MASTER, 0, 0, 0);
    if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGI(TAG, "I2C port %d already initialized", cfg->port);
        return ESP_OK;
    }

    return ret;
}

esp_err_t i2c_helper_deinit(i2c_port_t port)
{
    return i2c_driver_delete(port);
}

esp_err_t i2c_helper_write_byte(i2c_port_t port, uint8_t addr, uint8_t data)
{
    return i2c_helper_write(port, addr, &data, 1);
}

esp_err_t i2c_helper_write(i2c_port_t port, uint8_t addr,
                           const uint8_t *data, size_t len)
{
    if (!data || !len) return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, (uint8_t *)data, len, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t i2c_helper_read(i2c_port_t port, uint8_t addr,
                          uint8_t *data, size_t len)
{
    if (!data || !len) return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t i2c_helper_write_read(i2c_port_t port, uint8_t addr,
                                const uint8_t *wdata, size_t wlen,
                                uint8_t *rdata, size_t rlen)
{
    if (!wdata || !rdata) return ESP_ERR_INVALID_ARG;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, (uint8_t *)wdata, wlen, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, rdata, rlen, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return ret;
}
