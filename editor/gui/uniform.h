// Copyright (C) 2020-2021 Sami Väisänen
// Copyright (C) 2020-2021 Ensisoft http://www.ensisoft.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "config.h"

#include "warnpush.h"
#  include <QWidget>
#  include <glm/glm.hpp>
#include "warnpop.h"

#include "base/color4f.h"
#include "editor/app/types.h"

namespace Ui {
    class Uniform;
}

namespace gui
{
    // todo: rename to MultiValueWidget or something
    class Uniform : public QWidget
    {
        Q_OBJECT

    public:
        enum class Type {
            Int, Float, Vec2, Vec3, Vec4, Color, String
        };
        Uniform(QWidget* parent);
       ~Uniform();

        void SetType(Type type, QString suffix = "");
        void SetValue(int value);
        void SetValue(float value);
        void SetValue(const glm::vec2& value);
        void SetValue(const glm::vec3& value);
        void SetValue(const glm::vec4& value);
        void SetValue(const QColor& color);
        void SetValue(const base::Color4f& color);
        void SetValue(const QString& string);
        void SetValue(const std::string& str);
        float GetAsFloat() const;
        int GetAsInt() const;
        glm::vec2 GetAsVec2() const;
        glm::vec3 GetAsVec3() const;
        glm::vec4 GetAsVec4() const;
        QColor GetAsColor() const;
        QString GetAsString() const;
        void SetName(const app::AnyString& name)
        { mName = name; }
        app::AnyString GetName() const
        { return mName; }
        Type GetType() const
        { return mType;}

        void ShowIntAsCombo();
        void AddComboValue(const app::AnyString& str, int value);
        void SetComboValue(int value);
    private:
        void HideEverything();

    signals:
        void ValueChanged(const Uniform* uniform);
    private slots:
        void on_value_x_valueChanged(double);
        void on_value_y_valueChanged(double);
        void on_value_z_valueChanged(double);
        void on_value_w_valueChanged(double);
        void on_color_colorChanged(QColor);
        void on_string_editingFinished();
        void on_value_i_valueChanged(int);
        void on_combo_i_currentIndexChanged(int);
    private:
        Ui::Uniform* mUI = nullptr;
    private:
        app::AnyString mName;
        Type mType = Type::Int;
    };
} // namespace
