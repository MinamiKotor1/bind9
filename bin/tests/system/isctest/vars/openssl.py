# Copyright (C) Internet Systems Consortium, Inc. ("ISC")
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0.  If a copy of the MPL was not distributed with this
# file, you can obtain one at https://mozilla.org/MPL/2.0/.
#
# See the COPYRIGHT file distributed with this work for additional
# information regarding copyright ownership.

import os
import re

from .. import log


OPENSSL_VARS = {
    "OPENSSL_CONF": os.getenv("OPENSSL_CONF", ""),
    "SOFTHSM2_CONF": os.getenv("SOFTHSM2_CONF", ""),
    "SOFTHSM2_MODULE": "",
    "ENGINE_ARG": "",
}


def parse_openssl_config(path: str):
    if not os.path.isfile(path):
        return
    regex = re.compile(r"([^=]+)=(.*)")
    log.debug(f"parsing openssl config: {path}")
    with open(path, "r", encoding="utf-8") as conf:
        for line in conf:
            res = regex.match(line)
            if res:
                key = res.group(1).strip()
                val = res.group(2).strip()
                if key == "engine_id":
                    OPENSSL_VARS["ENGINE_ARG"] = f"-E {val}"
                    os.environ["ENGINE_ARG"] = f"-E {val}"
                    log.debug("ENGINE_ARG set to {OPENSSL_VARS['ENGINE_ARG']}")
                elif key in ["MODULE_PATH", "pkcs11-module-path"]:
                    OPENSSL_VARS["SOFTHSM2_MODULE"] = val
                    os.environ["SOFTHSM2_MODULE"] = val
                    log.debug(
                        "SOFTHSM2_MODULE set to {OPENSSL_VARS['SOFTHSM2_MODULE']}"
                    )


parse_openssl_config(OPENSSL_VARS["OPENSSL_CONF"])
