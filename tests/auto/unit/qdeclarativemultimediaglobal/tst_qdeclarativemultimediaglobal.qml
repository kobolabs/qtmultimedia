/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtTest 1.0
import QtMultimedia 5.4

TestCase {

    function test_0_globalObject() {
        verify(typeof QtMultimedia !== 'undefined');
    }

    function test_1_defaultCamera() {
        verify(typeof QtMultimedia.defaultCamera !== 'undefined');

        var camera = QtMultimedia.defaultCamera;
        compare(camera.deviceId, "othercamera", "deviceId");
        compare(camera.displayName, "othercamera desc", "displayName");
        compare(camera.position, Camera.UnspecifiedPosition, "position");
        compare(camera.orientation, 0, "orientation");
    }

    function test_2_availableCameras() {
        verify(typeof QtMultimedia.availableCameras !== 'undefined');
        compare(QtMultimedia.availableCameras.length, 2);

        var camera = QtMultimedia.availableCameras[0];
        compare(camera.deviceId, "backcamera", "deviceId");
        compare(camera.displayName, "backcamera desc", "displayName");
        compare(camera.position, Camera.BackFace, "position");
        compare(camera.orientation, 90, "orientation");

        camera = QtMultimedia.availableCameras[1];
        compare(camera.deviceId, "othercamera", "deviceId");
        compare(camera.displayName, "othercamera desc", "displayName");
        compare(camera.position, Camera.UnspecifiedPosition, "position");
        compare(camera.orientation, 0, "orientation");
    }

    function test_convertVolume_data() {
        return [
            { tag: "-1.0 from linear to linear", input: -1, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0 },
            { tag: "0.0 from linear to linear", input: 0, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0 },
            { tag: "0.5 from linear to linear", input: 0.5, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0.5 },
            { tag: "1.0 from linear to linear", input: 1.0, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 1.0 },

            { tag: "-1.0 from linear to cubic", input: -1, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0 },
            { tag: "0.0 from linear to cubic", input: 0, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0 },
            { tag: "0.33 from linear to cubic", input: 0.33, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0.69 },
            { tag: "0.5 from linear to cubic", input: 0.5, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0.79 },
            { tag: "0.72 from linear to cubic", input: 0.72, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0.89 },
            { tag: "1.0 from linear to cubic", input: 1.0, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 1.0 },

            { tag: "-1.0 from linear to decibel", input: -1, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -200 },
            { tag: "0.0 from linear to decibel", input: 0, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -200 },
            { tag: "0.33 from linear to decibel", input: 0.33, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -9.63 },
            { tag: "0.5 from linear to decibel", input: 0.5, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -6.02 },
            { tag: "0.72 from linear to decibel", input: 0.72, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -2.85 },
            { tag: "1.0 from linear to decibel", input: 1.0, from: QtMultimedia.LinearVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: 0 },

            { tag: "-1.0 from cubic to linear", input: -1, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0 },
            { tag: "0.0 from cubic to linear", input: 0, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0 },
            { tag: "0.33 from cubic to linear", input: 0.33, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0.04 },
            { tag: "0.5 from cubic to linear", input: 0.5, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0.13 },
            { tag: "0.72 from cubic to linear", input: 0.72, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0.37 },
            { tag: "1.0 from cubic to linear", input: 1.0, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 1 },

            { tag: "-1.0 from cubic to cubic", input: -1, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0 },
            { tag: "0.0 from cubic to cubic", input: 0, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0 },
            { tag: "0.5 from cubic to cubic", input: 0.5, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0.5 },
            { tag: "1.0 from cubic to cubic", input: 1.0, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 1.0 },

            { tag: "-1.0 from cubic to decibel", input: -1, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -200 },
            { tag: "0.0 from cubic to decibel", input: 0, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -200 },
            { tag: "0.33 from cubic to decibel", input: 0.33, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -28.89 },
            { tag: "0.5 from cubic to decibel", input: 0.5, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -18.06 },
            { tag: "0.72 from cubic to decibel", input: 0.72, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -8.56 },
            { tag: "1.0 from cubic to decibel", input: 1.0, from: QtMultimedia.CubicVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: 0 },

            { tag: "-1000 from decibel to linear", input: -1000, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0 },
            { tag: "-200 from decibel to linear", input: -200, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0 },
            { tag: "-40 from decibel to linear", input: -40, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0.01 },
            { tag: "-10 from decibel to linear", input: -10, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0.32 },
            { tag: "-5 from decibel to linear", input: -5, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 0.56 },
            { tag: "0 from decibel to linear", input: 0, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.LinearVolumeScale, expectedOutput: 1 },

            { tag: "-1000 from decibel to cubic", input: -1000, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0 },
            { tag: "-200 from decibel to cubic", input: -200, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0 },
            { tag: "-40 from decibel to cubic", input: -40, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0.22 },
            { tag: "-10 from decibel to cubic", input: -10, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0.68 },
            { tag: "-5 from decibel to cubic", input: -5, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 0.83 },
            { tag: "0 from decibel to cubic", input: 0, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.CubicVolumeScale, expectedOutput: 1 },

            { tag: "-1000 from decibel to decibel", input: -1000, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -1000 },
            { tag: "-200 from decibel to decibel", input: -200, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -200 },
            { tag: "-30 from decibel to decibel", input: -30, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: -30 },
            { tag: "0 from decibel to decibel", input: 0, from: QtMultimedia.DecibelVolumeScale, to: QtMultimedia.DecibelVolumeScale, expectedOutput: 0 },
        ]
    }

    function test_convertVolume(data) {
        fuzzyCompare(QtMultimedia.convertVolume(data.input, data.from, data.to),
                     data.expectedOutput,
                     0.01);
    }

}
