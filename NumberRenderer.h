#pragma once
#include "ScreenFixedBillboard.h"
#include <vector>
#include <string>

class NumberRenderer
{
private:
    // 静的メンバ：シーンをまたいで保持される数値
    int s_displayNumber;
    static bool s_isInitialized;

    // 最適化用：前回の数値を記録
    int m_lastDisplayNumber;
    std::vector<int> m_lastDigits; // 前回の各桁の値を記録

    // インスタンスメンバ：表示設定
    Vector2 m_basePosition;      // 基準位置（最初の桁の位置）
    float m_digitWidth;          // 1桁あたりの幅
    float m_digitHeight;         // 桁の高さ
    float m_digitSpacing;        // 桁間の間隔
    bool m_rightAlign;           // 右寄せかどうか

    // 数字スプライトシート関連
    ID3D11ShaderResourceView* m_digitTexture;
    static constexpr int DIGITS_PER_ROW = 10;  // 0-9で10個
    static constexpr float DIGIT_UV_WIDTH = 1.0f / DIGITS_PER_ROW;

    // 各桁のビルボード（動的に生成）
    std::vector<std::unique_ptr<ScreenFixedBillboard>> m_digitBillboards;

public:
    NumberRenderer();
    ~NumberRenderer();

    // 初期化・終了処理
    void Init(const Vector2& basePos, float digitWidth, float digitHeight,
        float spacing = 0.05f, bool rightAlign = true);
    void Dispose();

    // 静的メンバ操作（シーン間で共有される数値）
     void SetNumber(int number) { s_displayNumber = number; }
     int GetNumber() { return s_displayNumber; }
     void AddToNumber(int value) { s_displayNumber += value; }

    // 表示設定
    void SetPosition(const Vector2& pos) { m_basePosition = pos; }
    void SetDigitSize(float width, float height) { m_digitWidth = width; m_digitHeight = height; }
    void SetSpacing(float spacing) { m_digitSpacing = spacing; }
    void SetAlignment(bool rightAlign) { m_rightAlign = rightAlign; }

    // 更新・描画
    void Update();
    void Draw();

private:
    void LoadDigitTexture();
    void UpdateDigitBillboardsOptimized(); // 最適化版を追加
    void ResizeDigitBillboards(size_t newSize); // ビルボード数調整用
    void UpdateSingleDigit(size_t index, int digit, const Vector2& position); // 単一桁更新用
    Vector2 CalculateDigitUV(int digit);
    std::vector<int> NumberToDigits(int number);
};