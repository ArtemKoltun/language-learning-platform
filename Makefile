VENV := .venv
PYTHON := $(VENV)/bin/python
PIP := $(VENV)/bin/pip
NATIVE_OUT := backend/shared/native
BUILD := build

.PHONY: install install-editable build-cpp copy-native test-cpp test-py clean

install:
	python3.12 -m venv $(VENV)
	$(PIP) install --upgrade pip
	$(PIP) install pybind11

install-editable:
	$(PIP) install -e ".[dev]"

build-cpp:
	cmake -B $(BUILD) -S backend/native -DCMAKE_BUILD_TYPE=Release \
		-Dpybind11_DIR=$$($(PYTHON) -m pybind11 --cmakedir)
	cmake --build $(BUILD) --parallel

copy-native: build-cpp
	mkdir -p $(NATIVE_OUT)
	cp $(BUILD)/bindings/llp_core*.so $(NATIVE_OUT)/
	@echo "Copied llp_core to $(NATIVE_OUT)/"

test-cpp:
	ctest --test-dir $(BUILD) --output-on-failure

test-py:
	$(PYTHON) -m pytest tests/

clean:
	rm -rf $(BUILD)
	find $(NATIVE_OUT) -name "*.so" -delete