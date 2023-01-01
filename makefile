.PHONY: all configure compile test clean

all: compile

BUILD_DIR=build
TEST_RESULTS_DIR=test_results
FIRMWARE_BUILD_DIR=fw_build

configure:
	pip3 install meson ninja
	sudo apt-get install gcovr clang-format dos2unix

$(BUILD_DIR):
	meson setup --buildtype debug --optimization 2 --warnlevel 3 --werror -Db_coverage=true $(BUILD_DIR)

$(FIRMWARE_BUILD_DIR):
	meson setup --cross-file cross.txt --buildtype debug --optimization 2 --warnlevel 3 --werror -Db_coverage=true $(BUILD_DIR)

compile: $(BUILD_DIR)
	meson compile -C $(BUILD_DIR)

fw_compile: $(FIRMWARE_BUILD_DIR)
	meson compile -C $(FIRMWARE_BUILD_DIR)

test: compile
	build/unit_test
	for f in test/**/test.bats ; do (cd `dirname $$f`; pwd; bats test.bats); done

clean:
	rm -rf $(BUILD_DIR) $(TEST_RESULTS_DIR) html

tidy:
	clang-format --style=Microsoft -i inc/* src/* example/*

.PHONY: coverage_report
coverage_report:
	rm -rf coverage_report
	mkdir -p coverage_report
	gcovr --html-details --output coverage_report/index.html