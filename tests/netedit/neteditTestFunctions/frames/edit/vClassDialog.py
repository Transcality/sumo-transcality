# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.dev/sumo
# Copyright (C) 2009-2025 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    vClassDialog.py
# @author  Pablo Alvarez Lopez
# @date    28-05-25

# imports
from ...general.functions import *
from .boolAttribute import *


def modifyAttributeVClassDialog(attribute, vClass, disallowAll=True, cancel=False, reset=False):
    """
    @brief modify vclass attribute using dialog
    """
    # open dialog
    modifyBoolAttribute(attribute)
    # first check if disallow all
    if (disallowAll):
        for _ in range(attrs.dialog.allowVClass.disallowAll):
            typeKey('tab')
        typeKey('space')
        # go to vClass
        for _ in range(vClass - attrs.dialog.allowVClass.disallowAll):
            typeKey('tab')
        # Change current value
        typeKey('space')
    else:
        # go to vClass
        for _ in range(vClass):
            typeKey('tab')
        # Change current value
        typeKey('space')
    # check if cancel
    if (cancel):
        for _ in range(attrs.dialog.allowVClass.cancel - vClass):
            typeKey('tab')
        typeKey('space')
    elif (reset):
        for _ in range(attrs.dialog.allowVClass.reset - vClass):
            typeKey('tab')
        typeKey('space')
        for _ in range(2):
            typeTwoKeys('shift', 'tab')
        typeKey('space')
    else:
        for _ in range(attrs.dialog.allowVClass.accept - vClass):
            typeKey('tab')
        typeKey('space')


def modifyAttributeVClassDialogOverlapped(attribute, vClass, disallowAll=True, cancel=False, reset=False):
    """
    @brief modify vclass attribute using dialog
    """
    # open dialog
    modifyBoolAttributeOverlapped(attribute)
    # first check if disallow all
    if (disallowAll):
        for _ in range(attrs.dialog.allowVClass.disallowAll):
            typeKey('tab')
        typeKey('space')
        # go to vClass
        for _ in range(vClass - attrs.dialog.allowVClass.disallowAll):
            typeKey('tab')
        # Change current value
        typeKey('space')
    else:
        # go to vClass
        for _ in range(vClass):
            typeKey('tab')
        # Change current value
        typeKey('space')
    # check if cancel
    if (cancel):
        for _ in range(attrs.dialog.allowVClass.cancel - vClass):
            typeKey('tab')
        typeKey('space')
    elif (reset):
        for _ in range(attrs.dialog.allowVClass.reset - vClass):
            typeKey('tab')
        typeKey('space')
        for _ in range(2):
            typeTwoKeys('shift', 'tab')
        typeKey('space')
    else:
        for _ in range(attrs.dialog.allowVClass.accept - vClass):
            typeKey('tab')
        typeKey('space')
