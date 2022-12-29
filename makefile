.PHONY: all configure compile test clean

all: compile

BUILD_DIR=build
TEST_RESULTS_DIR=test_results
FIRMWARE_BUILD_DIR=fw_build

configure:
	pip3 install meson ninja
	sudo apt-get install gcovr clang-format dos2unix

test: compile
	rm -rf $(TEST_RESULTS_DIR)
	-mkdir $(TEST_RESULTS_DIR)
	bats --formatter junit --output $(TEST_RESULTS_DIR) test/test*.bats
	gcovr -s

$(BUILD_DIR):
	meson setup --buildtype debug --optimization 2 --warnlevel 3 --werror -Db_coverage=true $(BUILD_DIR)

$(FIRMWARE_BUILD_DIR):
	meson setup --cross-file cross.txt --buildtype debug --optimization 2 --warnlevel 3 --werror -Db_coverage=true $(BUILD_DIR)

compile: $(BUILD_DIR)
	meson compile -C $(BUILD_DIR)

fw_compile: $(FIRMWARE_BUILD_DIR)
	meson compile -C $(FIRMWARE_BUILD_DIR)


clean:
	rm -rf $(BUILD_DIR) $(TEST_RESULTS_DIR) html

tidy:
	clang-format --style=Microsoft -i inc/* src/* example/*

coverage_report:
	rm -rf html
	mkdir -p html
	gcovr --html-details --output html/index.html